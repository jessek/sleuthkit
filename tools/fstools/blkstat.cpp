/*
** blkstat
** The Sleuth Kit
**
** Get the details about a data unit
**
** Brian Carrier [carrier <at> sleuthkit [dot] org]
** Copyright (c) 2006-2011 Brian Carrier, Basis Technology.  All Rights reserved
** Copyright (c) 2003-2005 Brian Carrier.  All rights reserved
**
** TASK
** Copyright (c) 2002 Brian Carrier, @stake Inc.  All rights reserved
**
** This software is distributed under the Common Public License 1.0
**
*/
#include "tsk/tsk_tools_i.h"
#include <locale.h>

#include <memory>

static TSK_TCHAR *progname;

void
usage()
{
    TFPRINTF(stderr,
        _TSK_T
        ("usage: %" PRIttocTSK " [-vV] [-f fstype] [-i imgtype] [-b dev_sector_size] [-o imgoffset] [-P pooltype] [-B pool_volume_block] image [images] addr\n"),
        progname);
    tsk_fprintf(stderr,
        "\t-f fstype: File system type (use '-f list' for supported types)\n");
    tsk_fprintf(stderr, "\t-k password: Decryption password for encrypted volumes\n");
    tsk_fprintf(stderr,
        "\t-i imgtype: The format of the image file (use '-i list' for supported types)\n");
    tsk_fprintf(stderr,
        "\t-b dev_sector_size: The size (in bytes) of the device sectors\n");
    tsk_fprintf(stderr,
        "\t-o imgoffset: The offset of the file system in the image (in sectors)\n");
    tsk_fprintf(stderr,
        "\t-P pooltype: Pool container type (use '-P list' for supported types)\n");
    tsk_fprintf(stderr,
        "\t-B pool_volume_block: Starting block (for pool volumes only)\n");
    tsk_fprintf(stderr, "\t-v: Verbose output to stderr\n");
    tsk_fprintf(stderr, "\t-V: Print version\n");

    exit(1);
}

int
main(int argc, char **argv1)
{
    TSK_IMG_TYPE_ENUM imgtype = TSK_IMG_TYPE_DETECT;

    TSK_OFF_T imgaddr = 0;
    TSK_FS_TYPE_ENUM fstype = TSK_FS_TYPE_DETECT;

    TSK_POOL_TYPE_ENUM pooltype = TSK_POOL_TYPE_DETECT;
    TSK_OFF_T pvol_block = 0;
    const char * password = "";

    int ch;
    TSK_TCHAR *cp;
    extern int OPTIND;
    TSK_DADDR_T addr;
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

    while ((ch = GETOPT(argc, argv, _TSK_T("b:B:f:i:k:o:P:uvV"))) > 0) {
        switch (ch) {
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
                    _TSK_T("Unsupported pool container type: %s\n"), OPTARG);
                usage();
            }
            break;
        case _TSK_T('B'):
            if ((pvol_block = tsk_parse_offset(OPTARG)) == -1) {
                tsk_error_print(stderr);
                exit(1);
            }
            break;
        case _TSK_T('k'):
            password = argv1[OPTIND - 1];
            break;
        case _TSK_T('v'):
            tsk_verbose++;
            break;
        case _TSK_T('V'):
            tsk_version_print(stdout);
            exit(0);
        case _TSK_T('?'):
        default:
            TFPRINTF(stderr, _TSK_T("Invalid argument: %" PRIttocTSK "\n"),
                argv[OPTIND]);
            usage();
        }
    }

    if (OPTIND + 1 >= argc) {
        tsk_fprintf(stderr, "Missing image name and/or address\n");
        usage();
    }

    /* Get the address */
    addr = TSTRTOULL(argv[argc - 1], &cp, 0);
    if (*cp || *cp == *argv[argc - 1]) {
        tsk_fprintf(stderr, "Invalid address\n");
        usage();
    }

    /* open image */
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

    if (addr > fs->last_block) {
        tsk_fprintf(stderr,
            "Data unit address too large for image (%" PRIuDADDR ")\n",
            fs->last_block);
        exit(1);
    }
    if (addr < fs->first_block) {
        tsk_fprintf(stderr,
            "Data unit address too small for image (%" PRIuDADDR ")\n",
            fs->first_block);
        exit(1);
    }

    if (tsk_fs_blkstat(fs.get(), addr)) {
        tsk_error_print(stderr);
        exit(1);
    }

    exit(0);
}
