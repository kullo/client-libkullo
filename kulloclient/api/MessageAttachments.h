// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from api.djinni

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Kullo { namespace Api {

class AsyncTask;
class MessageAttachmentsContentListener;
class MessageAttachmentsSaveToListener;

class MessageAttachments {
public:
    virtual ~MessageAttachments() {}

    /** Returns all attachments for the given message */
    virtual std::vector<int64_t> allForMessage(int64_t msgId) = 0;

    /**
     * Returns true iff all attachments of the given message have been downloaded
     * (see Syncer for how to download attachments)
     */
    virtual bool allAttachmentsDownloaded(int64_t msgId) = 0;

    /** Filename (e.g. "hello.png") */
    virtual std::string filename(int64_t msgId, int64_t attId) = 0;

    /** MIME type (e.g. "image/png") */
    virtual std::string mimeType(int64_t msgId, int64_t attId) = 0;

    /** File size in bytes */
    virtual int64_t size(int64_t msgId, int64_t attId) = 0;

    /** SHA-512 hash of the attachment contents */
    virtual std::string hash(int64_t msgId, int64_t attId) = 0;

    /** Gets the content of the attachment as a BLOB */
    virtual std::shared_ptr<AsyncTask> contentAsync(int64_t msgId, int64_t attId, const std::shared_ptr<MessageAttachmentsContentListener> & listener) = 0;

    /**
     * Saves the content of the attachment to a file. Path contains the absolute
     * path where the file should be saved, including the filename.
     */
    virtual std::shared_ptr<AsyncTask> saveToAsync(int64_t msgId, int64_t attId, const std::string & path, const std::shared_ptr<MessageAttachmentsSaveToListener> & listener) = 0;
};

} }  // namespace Kullo::Api
