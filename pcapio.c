/* pcapio.c
 * Our own private code for writing libpcap files when capturing.
 *
 * We have these because we want a way to open a stream for output given
 * only a file descriptor.  libpcap 0.9[.x] has "pcap_dump_fopen()", which
 * provides that, but
 *
 *      1) earlier versions of libpcap doesn't have it
 *
 * and
 *
 *      2) WinPcap doesn't have it, because a file descriptor opened
 *         by code built for one version of the MSVC++ C library
 *         can't be used by library routines built for another version
 *         (e.g., threaded vs. unthreaded).
 *
 * Libpcap's pcap_dump() also doesn't return any error indications.
 *
 * $Id: pcapio.c 42242 2012-04-25 20:50:38Z tuexen $
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * Derived from code in the Wiretap Library
 * Copyright (c) 1998 by Gilbert Ramirez <gram@alumni.rice.edu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBPCAP

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <pcap.h>

#include <glib.h>

#include "pcapio.h"

/* Magic numbers in "libpcap" files.

   "libpcap" file records are written in the byte order of the host that
   writes them, and the reader is expected to fix this up.

   PCAP_MAGIC is the magic number, in host byte order; PCAP_SWAPPED_MAGIC
   is a byte-swapped version of that.

   PCAP_NSEC_MAGIC is for Ulf Lamping's modified "libpcap" format,
   which uses the same common file format as PCAP_MAGIC, but the
   timestamps are saved in nanosecond resolution instead of microseconds.
   PCAP_SWAPPED_NSEC_MAGIC is a byte-swapped version of that. */
#define PCAP_MAGIC                      0xa1b2c3d4
#define PCAP_SWAPPED_MAGIC              0xd4c3b2a1
#define PCAP_NSEC_MAGIC                 0xa1b23c4d
#define PCAP_SWAPPED_NSEC_MAGIC         0x4d3cb2a1

/* "libpcap" file header. */
struct pcap_hdr {
        guint32 magic;          /* magic number */
        guint16 version_major;  /* major version number */
        guint16 version_minor;  /* minor version number */
        gint32  thiszone;       /* GMT to local correction */
        guint32 sigfigs;        /* accuracy of timestamps */
        guint32 snaplen;        /* max length of captured packets, in octets */
        guint32 network;        /* data link type */
};

/* "libpcap" record header. */
struct pcaprec_hdr {
        guint32 ts_sec;         /* timestamp seconds */
        guint32 ts_usec;        /* timestamp microseconds (nsecs for PCAP_NSEC_MAGIC) */
        guint32 incl_len;       /* number of octets of packet saved in file */
        guint32 orig_len;       /* actual length of packet */
};

/* Magic numbers in ".pcapng" files.
 *
 * .pcapng file records are written in the byte order of the host that
 * writes them, and the reader is expected to fix this up.
 * PCAPNG_MAGIC is the magic number, in host byte order;
 * PCAPNG_SWAPPED_MAGIC is a byte-swapped version of that.
 */
#define PCAPNG_MAGIC         0x1A2B3C4D
#define PCAPNG_SWAPPED_MAGIC 0xD4C3B2A1

/* Currently we are only supporting the initial version of
   the file format. */
#define PCAPNG_MAJOR_VERSION 1
#define PCAPNG_MINOR_VERSION 0

/* Section Header Block without options and trailing Block Total Length */
struct shb {
        guint32 block_type;
        guint32 block_total_length;
        guint32 byte_order_magic;
        guint16 major_version;
        guint16 minor_version;
        guint64 section_length;
};
#define SECTION_HEADER_BLOCK_TYPE 0x0A0D0D0A

/* Interface Decription Block without options and trailing Block Total Length */
struct idb {
        guint32 block_type;
        guint32 block_total_length;
        guint16 link_type;
        guint16 reserved;
        guint32 snap_len;
};
#define INTERFACE_DESCRIPTION_BLOCK_TYPE 0x00000001

/* Interface Statistics Block without actual packet, options, and trailing
   Block Total Length */
struct isb {
        guint32 block_type;
        guint32 block_total_length;
        guint32 interface_id;
        guint32 timestamp_high;
        guint32 timestamp_low;
};
#define INTERFACE_STATISTICS_BLOCK_TYPE 0x00000005

/* Enhanced Packet Block without actual packet, options, and trailing
   Block Total Length */
struct epb {
        guint32 block_type;
        guint32 block_total_length;
        guint32 interface_id;
        guint32 timestamp_high;
        guint32 timestamp_low;
        guint32 captured_len;
        guint32 packet_len;
};
#define ENHANCED_PACKET_BLOCK_TYPE 0x00000006

struct option {
        guint16 type;
        guint16 value_length;
};
#define OPT_ENDOFOPT      0
#define OPT_COMMENT       1
#define SHB_HARDWARE      2 /* currently not used */
#define SHB_OS            3
#define SHB_USERAPPL      4
#define IDB_NAME          2
#define IDB_DESCRIPTION   3
#define IDB_IF_SPEED      8
#define IDB_TSRESOL       9
#define IDB_FILTER       11
#define IDB_OS           12
#define ISB_STARTTIME     2
#define ISB_ENDTIME       3
#define ISB_IFRECV        4
#define ISB_IFDROP        5
#define ISB_FILTERACCEPT  6
#define ISB_OSDROP        7
#define ISB_USRDELIV      8
#define ADD_PADDING(x) ((((x) + 3) >> 2) << 2)

#define WRITE_DATA(file_pointer, data_pointer, data_length, written_length, error_pointer) \
{                                                                                          \
        do {                                                                               \
                size_t nwritten;                                                           \
                                                                                           \
                nwritten = fwrite(data_pointer, 1, data_length, file_pointer);             \
                if (nwritten != data_length) {                                             \
                        if (nwritten == 0 && ferror(file_pointer)) {                       \
                                *error_pointer = errno;                                    \
                        } else {                                                           \
                                *error_pointer = 0;                                        \
                        }                                                                  \
                        return FALSE;                                                      \
                }                                                                          \
                written_length += (long)nwritten;                                          \
        } while (0);                                                                       \
}

/* Returns a FILE * to write to on success, NULL on failure */
FILE *
libpcap_fdopen(int fd, int *err)
{
        FILE *fp;

        fp = fdopen(fd, "wb");
        if (fp == NULL) {
                *err = errno;
        }
        return fp;
}

/* Write the file header to a dump file.
   Returns TRUE on success, FALSE on failure.
   Sets "*err" to an error code, or 0 for a short write, on failure*/
gboolean
libpcap_write_file_header(FILE *fp, int linktype, int snaplen, gboolean ts_nsecs, long *bytes_written, int *err)
{
        struct pcap_hdr file_hdr;
        size_t nwritten;

        file_hdr.magic = ts_nsecs ? PCAP_NSEC_MAGIC : PCAP_MAGIC;
        /* current "libpcap" format is 2.4 */
        file_hdr.version_major = 2;
        file_hdr.version_minor = 4;
        file_hdr.thiszone = 0;  /* XXX - current offset? */
        file_hdr.sigfigs = 0;   /* unknown, but also apparently unused */
        file_hdr.snaplen = snaplen;
        file_hdr.network = linktype;
        nwritten = fwrite(&file_hdr, 1, sizeof(file_hdr), fp);
        if (nwritten != sizeof(file_hdr)) {
                if (nwritten == 0 && ferror(fp))
                        *err = errno;
                else
                        *err = 0;       /* short write */
                return FALSE;
        }
        *bytes_written += sizeof(file_hdr);

        return TRUE;
}

/* Write a record for a packet to a dump file.
   Returns TRUE on success, FALSE on failure. */
gboolean
libpcap_write_packet(FILE *fp, const struct pcap_pkthdr *phdr, const u_char *pd,
    long *bytes_written, int *err)
{
        struct pcaprec_hdr rec_hdr;
        size_t nwritten;

        rec_hdr.ts_sec = phdr->ts.tv_sec;
        rec_hdr.ts_usec = phdr->ts.tv_usec;
        rec_hdr.incl_len = phdr->caplen;
        rec_hdr.orig_len = phdr->len;
        nwritten = fwrite(&rec_hdr, 1, sizeof rec_hdr, fp);
        if (nwritten != sizeof rec_hdr) {
                if (nwritten == 0 && ferror(fp))
                        *err = errno;
                else
                        *err = 0;       /* short write */
                return FALSE;
        }
        *bytes_written += sizeof rec_hdr;

        nwritten = fwrite(pd, 1, phdr->caplen, fp);
        if (nwritten != phdr->caplen) {
                if (nwritten == 0 && ferror(fp))
                        *err = errno;
                else
                        *err = 0;       /* short write */
                return FALSE;
        }
        *bytes_written += phdr->caplen;
        return TRUE;
}

gboolean
libpcap_write_session_header_block(FILE *fp,
                                   const char *comment,
                                   const char *hw,
                                   const char *os,
                                   const char *appname,
                                   guint64 section_length,
                                   long *bytes_written,
                                   int *err)
{
        struct shb shb;
        struct option option;
        guint32 block_total_length;
        const guint32 padding = 0;
        gboolean have_options = FALSE;

        /* Size of base header */
        block_total_length = sizeof(struct shb) +
                             sizeof(guint32);
        if ((comment != NULL) && (strlen(comment) > 0) && (strlen(comment) < G_MAXUINT16)) {
                block_total_length += sizeof(struct option) +
                                      (guint16)ADD_PADDING(strlen(comment));
                have_options = TRUE;
        }
        if ((hw != NULL) && (strlen(hw) > 0) && (strlen(hw) < G_MAXUINT16)) {
                block_total_length += sizeof(struct option) +
                                      (guint16)ADD_PADDING(strlen(hw));
                have_options = TRUE;
        }
        if ((os != NULL) && (strlen(os) > 0) && (strlen(os) < G_MAXUINT16)) {
                block_total_length += sizeof(struct option) +
                                      (guint16)ADD_PADDING(strlen(os));
                have_options = TRUE;
        }
        if ((appname != NULL) && (strlen(appname) > 0) && (strlen(appname) < G_MAXUINT16)) {
                block_total_length += sizeof(struct option) +
                                      (guint16)ADD_PADDING(strlen(appname));
                have_options = TRUE;
        }
        /* If we have options add size of end-of-options */
        if (have_options) {
                block_total_length += sizeof(struct option);
        }
        /* write shb header */
        shb.block_type = SECTION_HEADER_BLOCK_TYPE;
        shb.block_total_length = block_total_length;
        shb.byte_order_magic = PCAPNG_MAGIC;
        shb.major_version = PCAPNG_MAJOR_VERSION;
        shb.minor_version = PCAPNG_MINOR_VERSION;
        shb.section_length = section_length;
        WRITE_DATA(fp, &shb, sizeof(struct shb), *bytes_written, err);

        if ((comment != NULL) && (strlen(comment) > 0) && (strlen(comment) < G_MAXUINT16)) {
                /* write opt_comment options */
                option.type = OPT_COMMENT;
                option.value_length = (guint16)strlen(comment);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, comment, strlen(comment), *bytes_written, err);
                if (strlen(comment) % 4) {
                        WRITE_DATA(fp, &padding, 4 - strlen(comment) % 4, *bytes_written, err);
                }
        }
        if ((hw != NULL) && (strlen(hw) > 0) && (strlen(hw) < G_MAXUINT16)) {
                /* write shb_hardware options */
                option.type = SHB_HARDWARE;
                option.value_length = (guint16)strlen(hw);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, hw, strlen(hw), *bytes_written, err);
                if ((strlen(hw) + 1) % 4) {
                        WRITE_DATA(fp, &padding, 4 - strlen(hw) % 4, *bytes_written, err);
                }
        }
        if ((os != NULL) && (strlen(os) > 0) && (strlen(os) < G_MAXUINT16)) {
                /* write shb_os options */
                option.type = SHB_OS;
                option.value_length = (guint16)strlen(os);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, os, strlen(os), *bytes_written, err);
                if (strlen(os) % 4) {
                        WRITE_DATA(fp, &padding, 4 - strlen(os) % 4, *bytes_written, err);
                }
        }
        if ((appname != NULL) && (strlen(appname) > 0) && (strlen(appname) < G_MAXUINT16)) {
                /* write shb_userappl options */
                option.type = SHB_USERAPPL;
                option.value_length = (guint16)strlen(appname);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, appname, strlen(appname), *bytes_written, err);
                if (strlen(appname) % 4) {
                        WRITE_DATA(fp, &padding, 4 - strlen(appname) % 4, *bytes_written, err);
                }
        }
        if (have_options) {
                /* write end of options */
                option.type = OPT_ENDOFOPT;
                option.value_length = 0;
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
        }

        /* write the trailing block total length */
        WRITE_DATA(fp, &block_total_length, sizeof(guint32), *bytes_written, err);
        return TRUE;
}

gboolean
libpcap_write_interface_description_block(FILE *fp,
                                          const char *comment, /* OPT_COMMENT        1 */
                                          const char *name,    /* IDB_NAME           2 */
                                          const char *descr,   /* IDB_DESCRIPTION    3 */
                                          const char *filter,  /* IDB_FILTER        11 */
                                          const char *os,      /* IDB_OS            12 */
                                          int link_type,
                                          int snap_len,
                                          long *bytes_written,
                                          guint64 if_speed,    /* IDB_IF_SPEED       8 */
                                          guint8 tsresol,      /* IDB_TSRESOL        9 */
                                          int *err)
{
        struct idb idb;
        struct option option;
        guint32 block_total_length;
        const guint32 padding = 0;
        gboolean have_options = FALSE;

        block_total_length = sizeof(struct idb) + sizeof(guint32);
        /* 01 - OPT_COMMENT */
        if ((comment != NULL) && (strlen(comment) > 0) && (strlen(comment) < G_MAXUINT16)) {
                block_total_length += sizeof(struct option) +
                                      (guint16)ADD_PADDING(strlen(comment));
                have_options = TRUE;
        }

        /* 02 - IDB_NAME */
        if ((name != NULL) && (strlen(name) > 0) && (strlen(name) < G_MAXUINT16)) {
                block_total_length += sizeof(struct option) +
                                      (guint16)ADD_PADDING(strlen(name));
                have_options = TRUE;
        }

        /* 03 - IDB_DESCRIPTION */
        if ((descr != NULL) && (strlen(descr) > 0) && (strlen(descr) < G_MAXUINT16)) {
                block_total_length += sizeof(struct option) +
                                      (guint16)ADD_PADDING(strlen(descr));
                have_options = TRUE;
        }

        /* 08 - IDB_IF_SPEED */
        if (if_speed != 0) {
                block_total_length += sizeof(struct option) + sizeof(guint64);
                have_options = TRUE;
        }

        /* 09 - IDB_TSRESOL */
        if (tsresol != 0) {
                block_total_length += sizeof(struct option) + sizeof(struct option);
                have_options = TRUE;
        }

        /* 11 - IDB_FILTER */
        if ((filter != NULL) && (strlen(filter) > 0) && (strlen(filter) < G_MAXUINT16)) {
                block_total_length += sizeof(struct option) +
                                      (guint16)(ADD_PADDING(strlen(filter)+ 1));
                have_options = TRUE;
        }

        /* 12 - IDB_OS */
        if ((os != NULL) && (strlen(os) > 0) && (strlen(os) < G_MAXUINT16)) {
                block_total_length += sizeof(struct option) +
                                     (guint16)ADD_PADDING(strlen(os));
                have_options = TRUE;
        }

        /* If we have options add size of end-of-options */
        if (have_options) {
                block_total_length += sizeof(struct option);
        }

        /* write block header */
        idb.block_type = INTERFACE_DESCRIPTION_BLOCK_TYPE;
        idb.block_total_length = block_total_length;
        idb.link_type = link_type;
        idb.reserved = 0;
        idb.snap_len = snap_len;
        WRITE_DATA(fp, &idb, sizeof(struct idb), *bytes_written, err);

        /* 01 - OPT_COMMENT - write comment string if applicable */
        if ((comment != NULL) && (strlen(comment) > 0) && (strlen(comment) < G_MAXUINT16)) {
                option.type = OPT_COMMENT;
                option.value_length = (guint16)strlen(comment);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, comment, strlen(comment), *bytes_written, err);
                if (strlen(comment) % 4) {
                        WRITE_DATA(fp, &padding, 4 - strlen(comment) % 4 , *bytes_written, err);
                }
        }

        /* 02 - IDB_NAME - write interface name string if applicable */
        if ((name != NULL) && (strlen(name) > 0) && (strlen(name) < G_MAXUINT16)) {
                option.type = IDB_NAME;
                option.value_length = (guint16)strlen(name);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, name, strlen(name), *bytes_written, err);
                if (strlen(name) % 4) {
                        WRITE_DATA(fp, &padding, 4 - strlen(name) % 4 , *bytes_written, err);
                }
        }

        /* 03 - IDB_DESCRIPTION */
        /* write interface description string if applicable */
        if ((descr != NULL) && (strlen(descr) > 0) && (strlen(descr) < G_MAXUINT16)) {
                option.type = IDB_DESCRIPTION;
                option.value_length = (guint16)strlen(descr);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, descr, strlen(descr), *bytes_written, err);
                if (strlen(descr) % 4) {
                        WRITE_DATA(fp, &padding, 4 - strlen(descr) % 4 , *bytes_written, err);
                }
        }

        /* 08 - IDB_IF_SPEED */
        if (if_speed != 0) {
                option.type = IDB_IF_SPEED;
                option.value_length = sizeof(guint64);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, &if_speed, sizeof(guint64), *bytes_written, err);
        }

        /* 09 - IDB_TSRESOL */
        if (tsresol != 0) {
                option.type = IDB_TSRESOL;
                option.value_length = sizeof(guint8);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, &tsresol, sizeof(guint8), *bytes_written, err);
                WRITE_DATA(fp, &padding, 3 , *bytes_written, err);
        }

        /* 11 - IDB_FILTER - write filter string if applicable
         * We only write version 1 of the filter, libpcap string
         */
        if ((filter != NULL) && (strlen(filter) > 0) && (strlen(filter) < G_MAXUINT16)) {
                option.type = IDB_FILTER;
                option.value_length = (guint16)(strlen(filter) + 1 );
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);

                /* The first byte of the Option Data keeps a code of the filter used, 0 = lipbpcap filter string */
                WRITE_DATA(fp, &padding, 1, *bytes_written, err);
                WRITE_DATA(fp, filter, strlen(filter), *bytes_written, err);
                if ((strlen(filter) + 1) % 4) {
                        WRITE_DATA(fp, &padding, 4 - (strlen(filter) + 1) % 4 , *bytes_written, err);
                }
        }

        /* 12 - IDB_OS - write os string if applicable */
        if ((os != NULL) && (strlen(os) > 0) && (strlen(os) < G_MAXUINT16)) {
                option.type = IDB_OS;
                option.value_length = (guint16)strlen(os);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, os, strlen(os), *bytes_written, err);
                if (strlen(os) % 4) {
                        WRITE_DATA(fp, &padding, 4 - strlen(os) % 4 , *bytes_written, err);
                }
        }

        if (have_options) {
                /* write end of options */
                option.type = OPT_ENDOFOPT;
                option.value_length = 0;
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
        }

        /* write the trailing Block Total Length */
        WRITE_DATA(fp, &block_total_length, sizeof(guint32), *bytes_written, err);
        return TRUE;
}

/* Write a record for a packet to a dump file.
   Returns TRUE on success, FALSE on failure. */
gboolean
libpcap_write_enhanced_packet_block(FILE *fp,
                                    const struct pcap_pkthdr *phdr,
                                    guint32 interface_id,
                                    guint ts_mul,
                                    const u_char *pd,
                                    long *bytes_written,
                                    int *err)
{
        struct epb epb;
        guint32 block_total_length;
        guint64 timestamp;
        const guint32 padding = 0;

        block_total_length = sizeof(struct epb) +
                             ADD_PADDING(phdr->caplen) +
                             sizeof(guint32);
        timestamp = (guint64)(phdr->ts.tv_sec) * ts_mul +
                    (guint64)(phdr->ts.tv_usec);
        epb.block_type = ENHANCED_PACKET_BLOCK_TYPE;
        epb.block_total_length = block_total_length;
        epb.interface_id = interface_id;
        epb.timestamp_high = (guint32)((timestamp>>32) & 0xffffffff);
        epb.timestamp_low = (guint32)(timestamp & 0xffffffff);
        epb.captured_len = phdr->caplen;
        epb.packet_len = phdr->len;
        WRITE_DATA(fp, &epb, sizeof(struct epb), *bytes_written, err);
        WRITE_DATA(fp, pd, phdr->caplen, *bytes_written, err);
        if (phdr->caplen % 4) {
                WRITE_DATA(fp, &padding, 4 - phdr->caplen % 4, *bytes_written, err);
        }
        WRITE_DATA(fp, &block_total_length, sizeof(guint32), *bytes_written, err);
        return TRUE;
}

gboolean
libpcap_write_interface_statistics_block(FILE *fp,
                                         guint32 interface_id,
                                         long *bytes_written,
                                         const char *comment,   /* OPT_COMMENT           1 */
                                         guint64 isb_starttime, /* ISB_STARTTIME         2 */
                                         guint64 isb_endtime,   /* ISB_ENDTIME           3 */
                                         guint64 isb_ifrecv,    /* ISB_IFRECV            4 */
                                         guint64 isb_ifdrop,    /* ISB_IFDROP            5 */
                                         int *err)
{
        struct isb isb;
#ifdef _WIN32
        FILETIME now;
#else
        struct timeval now;
#endif
        struct option option;
        guint32 block_total_length;
        guint64 timestamp;
        gboolean have_options = FALSE;
        const guint32 padding = 0;
#ifdef _WIN32
        /*
         * Current time, represented as 100-nanosecond intervals since
         * January 1, 1601, 00:00:00 UTC.
         *
         * I think DWORD might be signed, so cast both parts of "now"
         * to guint32 so that the sign bit doesn't get treated specially.
         *
         * Windows 8 provides GetSystemTimePreciseAsFileTime which we
         * might want to use instead.
         */
        GetSystemTimeAsFileTime(&now);
        timestamp = (((guint64)(guint32)now.dwHighDateTime) << 32) +
                    (guint32)now.dwLowDateTime;

        /*
         * Convert to same thing but as 1-microsecond, i.e. 1000-nanosecond,
         * intervals.
         */
        timestamp /= 10;

        /*
         * Subtract difference, in microseconds, between January 1, 1601
         * 00:00:00 UTC and January 1, 1970, 00:00:00 UTC.
         */
        timestamp -= G_GINT64_CONSTANT(11644473600000000U);
#else
        /*
         * Current time, represented as seconds and microseconds since
         * January 1, 1970, 00:00:00 UTC.
         */
        gettimeofday(&now, NULL);

        /*
         * Convert to delta in microseconds.
         */
        timestamp = (guint64)(now.tv_sec) * 1000000 +
                    (guint64)(now.tv_usec);
#endif
        block_total_length = sizeof(struct isb) + sizeof(guint32);
        if (isb_ifrecv != G_MAXUINT64) {
            block_total_length +=  sizeof(struct option) + sizeof(guint64);
            have_options = TRUE;
        }
        if (isb_ifdrop != G_MAXUINT64) {
            block_total_length +=  sizeof(struct option) + sizeof(guint64);
            have_options = TRUE;
        }
        /* OPT_COMMENT */
        if ((comment != NULL) && (strlen(comment) > 0) && (strlen(comment) < G_MAXUINT16)) {
                block_total_length += sizeof(struct option) +
                                      (guint16)ADD_PADDING(strlen(comment));
                have_options = TRUE;
        }
        if (isb_starttime !=0) {
                block_total_length += sizeof(struct option) + sizeof(guint64); /* ISB_STARTTIME */
                have_options = TRUE;
        }
        if (isb_endtime !=0) {
                block_total_length += sizeof(struct option) + sizeof(guint64); /* ISB_ENDTIME */
                have_options = TRUE;
        }
        /* If we have options add size of end-of-options */
        if (have_options) {
                block_total_length += sizeof(struct option);
        }

        isb.block_type = INTERFACE_STATISTICS_BLOCK_TYPE;
        isb.block_total_length = block_total_length;
        isb.interface_id = interface_id;
        isb.timestamp_high = (guint32)((timestamp>>32) & 0xffffffff);
        isb.timestamp_low = (guint32)(timestamp & 0xffffffff);
        WRITE_DATA(fp, &isb, sizeof(struct isb), *bytes_written, err);

        /* write comment string if applicable */
        if ((comment != NULL) && (strlen(comment) > 0) && (strlen(comment) < G_MAXUINT16)) {
                option.type = OPT_COMMENT;
                option.value_length = (guint16)strlen(comment);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, comment, strlen(comment), *bytes_written, err);
                if (strlen(comment) % 4) {
                        WRITE_DATA(fp, &padding, 4 - strlen(comment) % 4 , *bytes_written, err);
                }
        }

        if (isb_starttime !=0) {
                guint32 high, low;

                option.type = ISB_STARTTIME;
                option.value_length = sizeof(guint64);
                high = (guint32)((isb_starttime>>32) & 0xffffffff);
                low = (guint32)(isb_starttime & 0xffffffff);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, &high, sizeof(guint32), *bytes_written, err);
                WRITE_DATA(fp, &low, sizeof(guint32), *bytes_written, err);
        }
        if (isb_endtime !=0) {
                guint32 high, low;

                option.type = ISB_ENDTIME;
                option.value_length = sizeof(guint64);
                high = (guint32)((isb_endtime>>32) & 0xffffffff);
                low = (guint32)(isb_endtime & 0xffffffff);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, &high, sizeof(guint32), *bytes_written, err);
                WRITE_DATA(fp, &low, sizeof(guint32), *bytes_written, err);
        }
        if (isb_ifrecv != G_MAXUINT64) {
                option.type = ISB_IFRECV;
                option.value_length = sizeof(guint64);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, &isb_ifrecv, sizeof(guint64), *bytes_written, err);
        }
        if (isb_ifdrop != G_MAXUINT64) {
                option.type = ISB_IFDROP;
                option.value_length = sizeof(guint64);
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
                WRITE_DATA(fp, &isb_ifdrop, sizeof(guint64), *bytes_written, err);
        }
        if (have_options) {
                /* write end of options */
                option.type = OPT_ENDOFOPT;
                option.value_length = 0;
                WRITE_DATA(fp, &option, sizeof(struct option), *bytes_written, err);
        }

        WRITE_DATA(fp, &block_total_length, sizeof(guint32), *bytes_written, err);

        return TRUE;
}

gboolean
libpcap_dump_flush(FILE *pd, int *err)
{
        if (fflush(pd) == EOF) {
                if (err != NULL)
                        *err = errno;
                return FALSE;
        }
        return TRUE;
}

gboolean
libpcap_dump_close(FILE *pd, int *err)
{
        if (fclose(pd) == EOF) {
                if (err != NULL)
                        *err = errno;
                return FALSE;
        }
        return TRUE;
}

#endif /* HAVE_LIBPCAP */

/*
 * Editor modelines  -  http://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 4
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=4 tabstop=8 expandtab:
 * :indentSize=4:tabSize=8:noTabs=true:
 */
