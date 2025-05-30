/*
** istat
** The Sleuth Kit
**
** Display all inode info about a given inode.
**
** Brian Carrier [carrier <at> sleuthkit [dot] org]
** Copyright (c) 2006-2011 Brian Carrier, Basis Technology.  All Rights reserved
** Copyright (c) 2003-2005 Brian Carrier.  All rights reserved
**
** TASK
** Copyright (c) 2002 Brian Carrier, @stake Inc.  All rights reserved
**
** TCTUTILs
** Copyright (c) 2001 Brian Carrier.  All rights reserved
**
**
** This software is distributed under the Common Public License 1.0
**
*/
#include "tsk/tsk_tools_i.h"
#include "tsk/fs/apfs_fs.h"
#include <locale.h>
#include <time.h>

#include <memory>

static TSK_TCHAR *progname;

/* usage - explain and terminate */
static void
usage()
{
    TFPRINTF(stderr,
        _TSK_T
        ("usage: %" PRIttocTSK " [-N num] [-f fstype] [-i imgtype] [-b dev_sector_size] [-o imgoffset] [-P pooltype] [-B pool_volume_block] [-z zone] [-s seconds] [-rvV] image inum\n"),
        progname);
    tsk_fprintf(stderr,
        "\t-N num: force the display of NUM address of block pointers\n");
    tsk_fprintf(stderr, "\t-r: display run list instead of list of block addresses\n");
    tsk_fprintf(stderr,
        "\t-z zone: time zone of original machine (i.e. EST5EDT or GMT)\n");
    tsk_fprintf(stderr,
        "\t-s seconds: Time skew of original machine (in seconds)\n");
    tsk_fprintf(stderr,
        "\t-i imgtype: The format of the image file (use '-i list' for supported types)\n");
    tsk_fprintf(stderr,
        "\t-b dev_sector_size: The size (in bytes) of the device sectors\n");
    tsk_fprintf(stderr,
        "\t-f fstype: File system type (use '-f list' for supported types)\n");
    tsk_fprintf(stderr,
        "\t-o imgoffset: The offset of the file system in the image (in sectors)\n");
    tsk_fprintf(stderr,
        "\t-P pooltype: Pool container type (use '-p list' for supported types)\n");
    tsk_fprintf(stderr,
        "\t-B pool_volume_block: Starting block (for pool volumes only)\n");
    tsk_fprintf(stderr, "\t-S snap_id: Snapshot ID (for APFS only)\n");
    tsk_fprintf(stderr, "\t-v: verbose output to stderr\n");
    tsk_fprintf(stderr, "\t-V: print version\n");
    tsk_fprintf(stderr, "\t-k password: Decryption password for encrypted volumes\n");
    exit(1);
}


int
main(int argc, char **argv1)
{
    TSK_IMG_TYPE_ENUM imgtype = TSK_IMG_TYPE_DETECT;

    TSK_OFF_T imgaddr = 0;
    TSK_FS_TYPE_ENUM fstype = TSK_FS_TYPE_DETECT;
    const char * password = "";

    TSK_POOL_TYPE_ENUM pooltype = TSK_POOL_TYPE_DETECT;
    TSK_OFF_T pvol_block = 0;
    TSK_OFF_T snap_id = 0;

    TSK_INUM_T inum;
    int ch;
    TSK_TCHAR *cp;
    int32_t sec_skew = 0;
    int istat_flags = 0;

    /* When > 0 this is the number of blocks to print, used for -B arg */
    TSK_DADDR_T numblock = 0;
    TSK_TCHAR **argv;
    unsigned int ssize = 0;

#ifdef TSK_WIN32
    // On Windows, get the wide arguments (mingw doesn't support wmain)
    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == NULL) {
        fprintf(stderr, "Error getting wide arguments\n");
        exit(1);
    }
#else
    argv = (TSK_TCHAR **) argv1;
#endif

    progname = argv[0];
    setlocale(LC_ALL, "");

    while ((ch = GETOPT(argc, argv, _TSK_T("b:N:f:i:o:rs:vVz:P:B:k:S:"))) > 0) {
        switch (ch) {
        case _TSK_T('?'):
        default:
            TFPRINTF(stderr, _TSK_T("Invalid argument: %" PRIttocTSK "\n"),
                argv[OPTIND]);
            usage();
            break;
        case _TSK_T('N'):
            numblock = TSTRTOULL(OPTARG, &cp, 0);
            if (*cp || *cp == *OPTARG || numblock < 1) {
                TFPRINTF(stderr,
                    _TSK_T
                    ("invalid argument: block count must be positive: %" PRIttocTSK "\n"),
                    OPTARG);
                usage();
            }
            break;
        case _TSK_T('b'):
            ssize = (unsigned int) TSTRTOUL(OPTARG, &cp, 0);
            if (*cp || *cp == *OPTARG || ssize < 1) {
                TFPRINTF(stderr,
                    _TSK_T
                    ("invalid argument: sector size must be positive: %" PRIttocTSK "\n"),
                    OPTARG);
                usage();
            }
            break;
        case _TSK_T('f'):
            if (TSTRCMP(OPTARG, _TSK_T("list")) == 0) {
                tsk_fs_type_print(stderr);
                exit(1);
            }
            fstype = tsk_fs_type_toid(OPTARG);
            if (fstype == TSK_FS_TYPE_UNSUPP) {
                TFPRINTF(stderr,
                    _TSK_T("Unsupported file system type: %" PRIttocTSK "\n"), OPTARG);
                usage();
            }
            break;
        case _TSK_T('k'):
            password = argv1[OPTIND - 1];
            break;
        case _TSK_T('i'):
            if (TSTRCMP(OPTARG, _TSK_T("list")) == 0) {
                tsk_img_type_print(stderr);
                exit(1);
            }
            imgtype = tsk_img_type_toid(OPTARG);
            if (imgtype == TSK_IMG_TYPE_UNSUPP) {
                TFPRINTF(stderr, _TSK_T("Unsupported image type: %" PRIttocTSK "\n"),
                    OPTARG);
                usage();
            }
            break;
        case _TSK_T('o'):
            if ((imgaddr = tsk_parse_offset(OPTARG)) == -1) {
                tsk_error_print(stderr);
                exit(1);
            }
            break;
        case _TSK_T('P'):
            if (TSTRCMP(OPTARG, _TSK_T("list")) == 0) {
                tsk_pool_type_print(stderr);
                exit(1);
            }
            pooltype = tsk_pool_type_toid(OPTARG);
            if (pooltype == TSK_POOL_TYPE_UNSUPP) {
                TFPRINTF(stderr,
                    _TSK_T("Unsupported pool container type: %" PRIttocTSK "\n"), OPTARG);
                usage();
            }
            break;
        case _TSK_T('B'):
            if ((pvol_block = tsk_parse_offset(OPTARG)) == -1) {
                tsk_error_print(stderr);
                exit(1);
            }
            break;
        case _TSK_T('S'):
            if ((snap_id = tsk_parse_offset(OPTARG)) == -1) {
                tsk_error_print(stderr);
                exit(1);
            }
            break;
        case _TSK_T('s'):
            sec_skew = TATOI(OPTARG);
            break;
        case _TSK_T('r'):
            istat_flags |= TSK_FS_ISTAT_RUNLIST;
            break;
        case _TSK_T('v'):
            tsk_verbose++;
            break;
        case _TSK_T('V'):
            tsk_version_print(stdout);
            exit(0);
        case _TSK_T('z'):
            {
                TSK_TCHAR envstr[32];
                TSNPRINTF(envstr, 32, _TSK_T("TZ=%s"), OPTARG);
                if (0 != TPUTENV(envstr)) {
                    tsk_fprintf(stderr, "error setting environment");
                    exit(1);
                }
                TZSET();
            }
            break;
        }
    }

    /* We need at least two more argument */
    if (OPTIND + 1 >= argc) {
        tsk_fprintf(stderr, "Missing image name and/or address\n");
        usage();
    }

    /* if we are given the inode in the inode-type-id form, then ignore
     * the other stuff w/out giving an error
     *
     * This will make scripting easier
     */
    if (tsk_fs_parse_inum(argv[argc - 1], &inum, NULL, NULL, NULL, NULL)) {
        TFPRINTF(stderr, _TSK_T("Invalid inode number: %" PRIttocTSK),
            argv[argc - 1]);
        usage();
    }

    /*
     * Open the file system.
     */
    std::unique_ptr<TSK_IMG_INFO, decltype(&tsk_img_close)> img{
        tsk_img_open(argc - OPTIND - 1, &argv[OPTIND], imgtype, ssize),
        tsk_img_close
    };

    if (!img) {
        tsk_error_print(stderr);
        exit(1);
    }

    if (imgaddr * img->sector_size >= img->size) {
        tsk_fprintf(stderr,
            "Sector offset supplied is larger than disk image (maximum: %"
            PRIu64 ")\n", img->size / img->sector_size);
        exit(1);
    }

    std::unique_ptr<const TSK_POOL_INFO, decltype(&tsk_pool_close)> pool{
        nullptr,
        tsk_pool_close
    };

    std::unique_ptr<TSK_IMG_INFO, decltype(&tsk_img_close)> pool_img{
        nullptr,
        tsk_img_close
    };

    std::unique_ptr<TSK_FS_INFO, decltype(&tsk_fs_close)> fs{
        nullptr,
        tsk_fs_close
    };

    if (pvol_block == 0) {
        fs.reset(tsk_fs_open_img_decrypt(img.get(), imgaddr * img->sector_size, fstype, password));
         if (!fs) {
            tsk_error_print(stderr);
            if (tsk_error_get_errno() == TSK_ERR_FS_UNSUPTYPE)
                tsk_fs_type_print(stderr);
            exit(1);
        }
    }
    else {
        pool.reset(tsk_pool_open_img_sing(img.get(), imgaddr * img->sector_size, pooltype));
        if (!pool) {
            tsk_error_print(stderr);
            if (tsk_error_get_errno() == TSK_ERR_FS_UNSUPTYPE)
                tsk_pool_type_print(stderr);
            exit(1);
        }

        TSK_OFF_T offset = imgaddr * img->sector_size;
#if HAVE_LIBVSLVM
        if (pool->ctype == TSK_POOL_TYPE_LVM){
            offset = 0;
        }
#endif /* HAVE_LIBVSLVM */
        pool_img.reset(pool->get_img_info(pool.get(), (TSK_DADDR_T)pvol_block));
        // FIXME: Will crash if !pool_img
        fs.reset(tsk_fs_open_img_decrypt(pool_img.get(), offset, fstype, password));
        if (!fs) {
            tsk_error_print(stderr);
            if (tsk_error_get_errno() == TSK_ERR_FS_UNSUPTYPE)
                tsk_fs_type_print(stderr);
            exit(1);
        }
    }

    if (inum > fs->last_inum) {
        tsk_fprintf(stderr,
            "Metadata address is too large for image (%" PRIuINUM ")\n",
            fs->last_inum);
        exit(1);
    }

    if (inum < fs->first_inum) {
        tsk_fprintf(stderr,
            "Metadata address is too small for image (%" PRIuINUM ")\n",
            fs->first_inum);
        exit(1);
    }

    if (snap_id > 0) {
      tsk_apfs_set_snapshot(fs.get(), (uint64_t)snap_id);
    }

    if (fs->istat(fs.get(), (TSK_FS_ISTAT_FLAG_ENUM) istat_flags, stdout, inum, numblock, sec_skew)) {
        tsk_error_print(stderr);
        exit(1);
    }

    exit(0);
}
