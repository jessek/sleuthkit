/*
 *
 *  The Sleuth Kit
 *
 *  Contact: Brian Carrier [carrier <at> sleuthkit [dot] org]
 *  Copyright (c) 2013 Basis Technology Corporation. All Rights
 *  reserved.
 *
 *  This software is distributed under the Common Public License 1.0
 */

/**
 * \file
 * 
 */

#include <iostream>
#include <istream>
#include <sstream>
#include <algorithm>

// Poco includes
#include "Poco/SharedPtr.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"

// Framework includes
#include "framework_i.h" // to get TSK_FRAMEWORK_API
#include "TskL01Extract.h"
//#include "TskAutoImpl.h"
#include "Services/TskServices.h"
#include "Utilities/TskUtilities.h"
#include "tsk3/base/tsk_base_i.h"

#ifndef HAVE_LIBEWF
#define HAVE_LIBEWF 1
#endif

namespace ewf
{
    #include "ewf.h"
}


TskL01Extract::TskL01Extract() :
    m_db(TskServices::Instance().getImgDB()),
    m_img_info(NULL)
{
}

TskL01Extract::~TskL01Extract()
{
    close();
}


void TskL01Extract::close()
{
    if (m_img_info)
    {
        tsk_img_close(m_img_info);
        m_img_info = NULL;
    }

    std::vector<ArchivedFile>::iterator it = m_archivedFiles.begin();
    for (; it != m_archivedFiles.end(); ++it)
    {
        delete [] it->dataBuf;
    }

    m_archivePath.clear();
}

/*
    If parent is NULL, then we don't use that as a source for paths and we set the parent ID to 0.
 */
///@todo use m_parentFile if it's not null
int TskL01Extract::extractFiles(const std::wstring &archivePath, TskFile * parent /*= NULL*/)
{
    m_archivePath = archivePath;
    m_parentFile = parent;

    static const std::string MSG_PREFIX = "TskL01Extract::extractFiles : ";
    try
    {
        if (archivePath.empty() && (parent == NULL))
        {
            throw TskException(MSG_PREFIX + "No path to archive provided.");
        }

        if (openContainer() != 0)
        {
            return -1;
        }

        if (m_img_info == NULL) {
            throw TskException(MSG_PREFIX +"Images not open yet");
        }

        TskImgDB& imgDB = TskServices::Instance().getImgDB();
		// Create a map of directory names to file ids to use to 
		// associate files/directories with the correct parent.
		//std::map<std::string, uint64_t> directoryMap;
        uint64_t parentId = 0;
#if 0
        m_db.addImageInfo((int)m_img_info->itype, m_img_info->sector_size);

        char *img_ptr = NULL;
#ifdef TSK_WIN32
        char img2[1024];
        UTF8 *ptr8;
        UTF16 *ptr16;

        ptr8 = (UTF8 *) img2;
        ptr16 = (UTF16 *) m_archivePath.c_str();

        TSKConversionResult retval =
            tsk_UTF16toUTF8_lclorder((const UTF16 **) &ptr16, (UTF16 *)
            & ptr16[wcslen(m_archivePath.c_str()) + 1], &ptr8,
            (UTF8 *) ((uintptr_t) ptr8 + 1024), TSKlenientConversion);
        if (retval != TSKconversionOK) 
        {
            throw TskException("TskL01Extract::extractFiles: Error converting image to UTF-8");
        }
        img_ptr = img2;
#else
        img_ptr = (char *) m_archivePath;
#endif

        m_db.addImageName(img_ptr);
#endif

        std::vector<ArchivedFile>::iterator it = m_archivedFiles.begin();
        for (; it != m_archivedFiles.end(); ++it)
        {
            if (it->type == 'f')
            {
                Poco::Path path(it->name);
                Poco::Path parent = path.parent();
                std::string name;

                if (path.isDirectory())
                    name = path[path.depth() - 1];
                else
                    name = path[path.depth()];

                ///@todo create a tskfile for the L01 file?

                // Determine the parent id of the file.
                //if (path.depth() == 0 || path.isDirectory() && path.depth() == 1)
                //    // This file or directory lives at the root so our parent id
                //    // is the containing file id.
                //    parentId = pFile->getId();
                //else
                //{
                //    // We are not at the root so we need to lookup the id of our
                //    // parent directory.
                //    std::map<std::string, uint64_t>::const_iterator pos;
                //    pos = directoryMap.find(parent.toString());

                //    if (pos == directoryMap.end())
                //    {
                //        //parentId = getParentIdForPath(parent, pFile->getId(), pFile->getFullPath(), directoryMap);
                //    }
                //    else
                //    {
                //        parentId = pos->second;
                //    }
                //}

                // Store some extra details about the derived (i.e, extracted) file.
                std::stringstream details;

                uint64_t fileId;

                std::string fullpath = "";
                //fullpath.append(pFile->getFullPath());
                //fullpath.append("\\");
                fullpath.append(path.toString());
                ///@todo file timestamp?
                if (imgDB.addDerivedFileInfo(name,
                    parentId,
                    path.isDirectory(),
                    it->size,
                    details.str(), 
                    0, // ctime
                    0, // crtime
                    0, // atime
                    0, //utc time
                    fileId, fullpath) == -1) 
                {
                        std::wstringstream msg;
                        msg << L"addDerivedFileInfo failed for name="
                            << name.c_str();
                        LOGERROR(msg.str());
                }

                // For file nodes, recreate file locally
                if (it->dataBuf != NULL)
                {
                    saveFile(fileId, *it);
                }

                // Schedule
                imgDB.updateFileStatus(fileId, TskImgDB::IMGDB_FILES_STATUS_READY_FOR_ANALYSIS);
            }
        }

        ///@todo dev test
        std::cerr << "Done extracting!\n";
    }
    catch (TskException &ex)
    {
        std::ostringstream msg;
        msg << MSG_PREFIX << "TskException: " << ex.message();
        LOGERROR(msg.str());
        return -1;
    }
    catch (std::exception &ex)
    {
        std::ostringstream msg;
        msg << MSG_PREFIX << "std::exception: " << ex.what();
        LOGERROR(msg.str());
        return -1;
    }
    catch (...)
    {
        LOGERROR(MSG_PREFIX + "unrecognized exception");
        return -1;
    }

    return 0;
}


int TskL01Extract::openContainer()
{
    static const std::string MSG_PREFIX = "TskL01Extract::openContainer : ";
    ewf::libewf_error_t *ewfError = NULL;
    try
    {
        if (m_archivePath.empty())
        {
            throw TskException("Error: archive path is empty.");
        }

        m_img_info = tsk_img_open_sing(m_archivePath.c_str(), TSK_IMG_TYPE_EWF_EWF, 512);
        if (m_img_info == NULL) 
        {
            std::stringstream logMessage;
            logMessage << "Error with tsk_img_open: " << tsk_error_get() << std::endl;
            throw TskException(logMessage.str());
        }

        /// TSK stores different struct objs to the same pointer
        ///@todo does C++ <> cast work on this?
        ewf::IMG_EWF_INFO *ewfInfo = (ewf::IMG_EWF_INFO*)m_img_info;
        m_img_info = &(ewfInfo->img_info);

        ewf::libewf_file_entry_t *root = NULL;
        int ret = ewf::libewf_handle_get_root_file_entry(ewfInfo->handle, &root, &ewfError);
        if (ret == -1)
        {
            std::stringstream logMessage;
            logMessage << "TskL01Extract::openContainers - Error with libewf_handle_get_root_file_entry: ";
            throw TskException(logMessage.str());
        }

        if (ret > 0)
        {
            ewf::uint8_t nameString[512];
            nameString[0] = '\0';
            ewfError = NULL;
            if (ewf::libewf_file_entry_get_utf8_name(root, nameString, 512, &ewfError) == -1)
            {
                std::stringstream logMessage;
                logMessage << "Error with libewf_file_entry_get_utf8_name: ";
                throw TskException(logMessage.str());
            }

            traverse(root);
        }
    }
    catch (TskException &ex)
    {
        std::ostringstream msg;
        msg << MSG_PREFIX << "TskException: " << ex.message();
        if (ewfError)
        {
            char errorString[512];
            errorString[0] = '\0';
            ewf::libewf_error_backtrace_sprint(ewfError, errorString, 512);
            msg << "libewf error: " << errorString << std::endl;
        }
        LOGERROR(msg.str());
        return -1;
    }
    catch (std::exception &ex)
    {
        std::ostringstream msg;
        msg << MSG_PREFIX << "std::exception: " << ex.what();
        LOGERROR(msg.str());
        return -1;
    }
    catch (...)
    {
        LOGERROR(MSG_PREFIX + "unrecognized exception");
        return -1;
    }
    /// dev testing ////
    //return -1;
    return 0;
}

/*
    Traverse the hierarchy inside the container

    ///@todo This puts the entire size of the uncompressed archive onto the heap,
    which might be bad for large files.
 */
void TskL01Extract::traverse(ewf::libewf_file_entry_t *parent)
{
    TskL01Extract::ArchivedFile fileInfo;
    fileInfo.entry   = parent;
    fileInfo.name    = getName(parent);
    fileInfo.type    = getFileType(parent);
    fileInfo.size    = getFileSize(parent);
    fileInfo.dataBuf = getFileData(parent, fileInfo.size);

    m_archivedFiles.push_back(fileInfo);

    int num = 0;
    ewf::libewf_error_t *ewfError = NULL;
    ewf::libewf_file_entry_get_number_of_sub_file_entries(parent, &num, &ewfError);

    ///@todo remove dev debug prints
    std::cerr << "number of sub file entries = " << num << std::endl;

    if (num > 0)
    {
        //recurse
        for (int i=0; i < num; ++i)
        {
            std::cerr << "traversing child " << i << std::endl;
            ewf::libewf_file_entry_t *child = NULL;
            ewfError = NULL;
            if (ewf::libewf_file_entry_get_sub_file_entry(parent, i, &child, &ewfError) == -1)
            {
                throw TskException("TskL01Extract::traverse - Error with libewf_file_entry_get_sub_file_entry: ");
            }

            traverse(child);
        }
    }
}


const std::string TskL01Extract::getName(ewf::libewf_file_entry_t *node)
{
    ///@todo
    //libewf_file_entry_get_utf8_name_size

    ewf::uint8_t nameString[512];
    nameString[0] = '\0';
    ewf::libewf_error_t *ewfError = NULL;
    if (ewf::libewf_file_entry_get_utf8_name(node, nameString, 512, &ewfError) == -1)
    {
        std::stringstream logMessage;
        char errorString[512];
        errorString[0] = '\0';
        ewf::libewf_error_backtrace_sprint(ewfError, errorString, 512);
        logMessage << "TskL01Extract::getName - Error with libewf_file_entry_get_utf8_name: " << errorString << std::endl;
        throw TskException(logMessage.str());
    }
    std::cerr << "File name = " << nameString << std::endl;
    std::string s;
    s.assign((char*)&nameString[0]);
    return s;
}


const ewf::uint8_t TskL01Extract::getFileType(ewf::libewf_file_entry_t *node)
{
    ewf::uint8_t type = 0;
    ewf::libewf_error_t *ewfError = NULL;
    if (ewf::libewf_file_entry_get_type(node, &type, &ewfError) == -1)
    {
        throw TskException("TskL01Extract::getFileType - Error with libewf_file_entry_get_utf8_name: ");
    }

    ewf::uint32_t flags = 0;
    ewfError = NULL;
    if (ewf::libewf_file_entry_get_flags(node, &flags, &ewfError) == -1)
    {
        throw TskException("TskL01Extract::getFileType - Error with libewf_file_entry_get_flags: ");
    }

    std::cerr << "File type = " << type << std::endl;
    std::cerr << "File flags = " << flags << std::endl;
    return type;
}


const uint64_t TskL01Extract::getFileSize(ewf::libewf_file_entry_t *node)
{
    ewf::size64_t fileSize = 0;
    ewf::libewf_error_t *ewfError = NULL;
    if (ewf::libewf_file_entry_get_size(node, &fileSize, &ewfError) == -1)
    {
        std::stringstream logMessage;
        char errorString[512];
        errorString[0] = '\0';
        ewf::libewf_error_backtrace_sprint(ewfError, errorString, 512);
        logMessage << "TskL01Extract::getFileSize - Error with libewf_file_entry_get_utf8_name: " << errorString << std::endl;
        throw TskException(logMessage.str());
    }
    std::cerr << "File size = " << (int)fileSize << std::endl;
    return fileSize;
}


char * TskL01Extract::getFileData(ewf::libewf_file_entry_t *node, const size_t dataSize)
{
    if (dataSize > 0)
    {
        //Poco::SharedPtr<unsigned char, Poco::ReferenceCounter, ArrayReleasePolicy> buffer(new unsigned char[dataSize]);
        char *buffer = new char[dataSize];
        ewf::libewf_error_t *ewfError = NULL;
        ewf::ssize_t bytesRead = ewf::libewf_file_entry_read_buffer(node, buffer, dataSize, &ewfError);
        if (bytesRead == -1)
        {
            std::stringstream logMessage;
            char errorString[512];
            errorString[0] = '\0';
            ewf::libewf_error_backtrace_sprint(ewfError, errorString, 512);
            logMessage << "TskL01Extract::getFileData - Error with libewf_file_entry_read_buffer: " << errorString << std::endl;
            LOGERROR(logMessage.str());
            return NULL;
        }
        std::cerr << "Data bytes read = " << (int)bytesRead<< std::endl;

        return buffer;
    }
    return NULL;
}


/* Create an uncompressed version of the file on the local file system.
 */
void TskL01Extract::saveFile(const uint64_t fileId, const ArchivedFile &archivedFile)
{
    try
    {
        BufStreamBuf b(archivedFile.dataBuf, archivedFile.dataBuf + archivedFile.size);
        std::istream in(&b);

        TskServices::Instance().getFileManager().addFile(fileId, in);
    }
    catch (Poco::Exception& ex)
    {
        std::wstringstream msg;
        msg << L"TskL01Extract::addFile - Error saving file from stream : " << ex.displayText().c_str();
        LOGERROR(msg.str());
        throw TskFileException("Error saving file from stream.");
    }
}
