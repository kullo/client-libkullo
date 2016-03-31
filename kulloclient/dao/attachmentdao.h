/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <cstdint>
#include <istream>
#include <memory>
#include <string>
#include <vector>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/dao/enums.h"
#include "kulloclient/dao/result.h"

namespace Kullo {
namespace Dao {

typedef Result<AttachmentDao> AttachmentResult;

/**
 * @brief Data Access Object for message/draft attachments
 */
class AttachmentDao
{
public:
    /**
     * @brief Create a new attachment.
     * @param draft Whether the attachment should be attached to a draft.
     */
    explicit AttachmentDao(Db::SharedSessionPtr session);

    /**
     * @brief Load an attachment from DB or create a new one if it doesn't exist.
     * @param draft Whether a draft attachment should be loaded or created.
     * @param messageId Message ID if draft == false, conversation ID otherwise.
     * @param index Zero-based index for ordering attachments inside a draft or message.
     */
    static std::unique_ptr<AttachmentDao> load(IsDraft draft, id_type messageId, id_type index, Db::SharedSessionPtr session);

    /**
     * @brief Get the message ID for the first not yet downloaded attachment.
     * @return -1 if there's no such message, message ID otherwise
     */
    static int64_t msgIdForFirstDownloadable(Db::SharedSessionPtr session);

    /**
     * @brief Get a list of attachments for a given message or draft.
     * @param draft Whether draft attachments should be loaded.
     * @param messageId Message ID if draft == false, conversation ID otherwise.
     */
    static std::unique_ptr<AttachmentResult> allForMessage(IsDraft draft, id_type messageId, Db::SharedSessionPtr session);

    /**
     * @brief Get a list of all message or draft attachments.
     * @param draft Whether draft attachments should be loaded.
     */
    static std::unique_ptr<AttachmentResult> all(
            IsDraft draft,
            Db::SharedSessionPtr session);

    /**
     * @brief Returns false iff there are attachments of the given message that
     *        have not yet been downloaded.
     */
    static bool allAttachmentsDownloaded(
            int64_t msgId, Db::SharedSessionPtr session);

    /**
     * @brief Saves the current state to DB if the state was changed.
     * @return true if state was dirty
     */
    bool save();

    void convertToMessageAttachment(id_type msgId);

    bool draft() const;

    bool setDraft(bool draft);

    /// Get the message ID (iff draft == false) or conversation ID (iff draft == true)
    id_type messageId() const;

    /// Set the message ID
    bool setMessageId(id_type messageId);

    /// Get the attachment index
    id_type index() const;

    /// Set the attachment index
    bool setIndex(id_type index);

    /// Get the filename of the attachment (e.g. "contract.pdf")
    std::string filename() const;

    /// Set the filename of the attachment
    bool setFilename(const std::string &filename);

    /// Get the MIME type (e.g. "application/pdf")
    std::string mimeType() const;

    /// Set the MIME type
    bool setMimeType(const std::string &mimeType);

    /// Get the attachment size in bytes
    std::size_t size() const;

    /// Set the attachment size in bytes
    bool setSize(std::size_t size);

    /// Get the SHA-512 hash of the content as a hex string
    std::string hash() const;

    /// Set the SHA-512 hash of the content as a hex string
    bool setHash(const std::string &hash);

    /**
     * @brief Get the binary attachment contents
     * @throw NotFoundException if the attachment data could not be found.
     */
    std::vector<unsigned char> content() const;

    /// Save content to ostream
    void saveContent(std::ostream &stream) const;

    /**
     * @brief Set the binary attachment contents
     * @throw NotFoundException if the attachment record could not be found.
     */
    void setContent(const std::vector<unsigned char> &content);

    /**
     * @brief Loads the attachment content from a stream. Size must have been
     * set previously.
     * @param input the input stream
     */
    void setContent(std::istream &input);

    /// Get the textual note that is attached to each attachment
    std::string note() const;

    /// Set the textual note that is attached to each attachment
    bool setNote(const std::string &note);

    /// Returns true iff the attachment has been deleted.
    bool deleted() const;

    /**
     * @brief Deletes a message's or draft's attachment, which cannot be undone.
     */
    void deletePermanently();

    /**
     * @brief Get the next free ID that can be used for a draft attachment.
     * @param conversationId The ID of the conversation that should get an additional attachment.
     */
    static id_type idForNewDraftAttachment(id_type conversationId, Db::SharedSessionPtr session);

    /**
     * @brief Delete all attachments in a draft.
     * @param conversationId ID of the conversation that the draft belongs to.
     */
    static std::vector<id_type> deleteAttachmentsForDraft(id_type conversationId, Db::SharedSessionPtr session);

    /**
     * @brief Delete all attachments of a message.
     * @param messageId ID of the message whose attachments should be deleted.
     */
    static std::vector<id_type> deleteAttachmentsForMessage(id_type messageId, Db::SharedSessionPtr session);

private:
    static std::vector<id_type> deleteAttachments(IsDraft draft, id_type messageId, Db::SharedSessionPtr session);
    static std::unique_ptr<AttachmentDao> loadFromDb(const SmartSqlite::Row &row, Db::SharedSessionPtr session);

    Db::SharedSessionPtr session_;
    bool draft_ = false, dirty_ = true, storedInDb_ = false, deleted_ = false;
    id_type messageId_ = 0;
    id_type index_ = 0;
    std::size_t size_ = 0;
    std::string filename_, mimeType_, hash_, note_;

    friend class Result<AttachmentDao>;
};

}
}
