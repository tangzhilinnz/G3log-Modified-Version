#include "g3log/filerotatewithfiltersink.hpp"

#include <memory>
#include <algorithm>


namespace g3 {
    using namespace internal;

    // helper function to create an logging sink with filter
    std::unique_ptr<FileRotateWithFilter> FileRotateWithFilter::CreateLogRotateWithFilter(
        std::string filename, std::string directory, std::vector<LEVELS> filter) {
        auto fileRotateUptr = std::make_unique<FileRotateSink>(filename, directory);
        return std::make_unique<FileRotateWithFilter>(std::move(fileRotateUptr), filter);
    }

    /// @param logToFile rotate file logging sink
    /// @param removes all log entries with LEVELS in this filter
    FileRotateWithFilter::FileRotateWithFilter(FileRotateUptr logToFile, IgnoreLogLevels ignoreLevels)
        : logger_(std::move(logToFile))
        , filter_(std::move(ignoreLevels))
    {}

    FileRotateWithFilter::~FileRotateWithFilter() {}

    void FileRotateWithFilter::setMaxArchiveLogCount(int max_size) {
        logger_->setMaxArchiveLogCount(max_size);
    }

    int FileRotateWithFilter::getMaxArchiveLogCount() {
        return logger_->getMaxArchiveLogCount();
    }

    void FileRotateWithFilter::setMaxLogSize(int max_file_size) {
        logger_->setMaxLogSize(max_file_size);
    }

    int FileRotateWithFilter::getMaxLogSize() {
        return logger_->getMaxLogSize();
    }

    /// @param logEntry saves log entry that are not in the filter
    void FileRotateWithFilter::save(LogMessageMover logEntry) {
        auto level = logEntry.get()._level;
        bool isNotInFilter_ = (filter_.end() == std::find(filter_.begin(), filter_.end(), level));

        if (isNotInFilter_) {
            //logger_->save(logEntry.get().toString(_log_details_func));
            logger_->fileWrite(logEntry);
        }
    }

    /**
     * Flush policy: Default is every single time (i.e. policy of 1).
     *
     * If the system logs A LOT then it is likely better to allow for the system to buffer and write
     * all the entries at once.
     *
     * 0: System decides, potentially very long time
     * 1....N: Flush logs every n entry
     */
    void FileRotateWithFilter::setFlushPolicy(size_t flush_policy) {
        logger_->setFlushPolicy(flush_policy);
    }

    /**
     * Force flush of log entries. This should normally be policed with the @ref setFlushPolicy
     * but is great for unit testing and if there are special circumstances where you want to see
     * the logs faster than the flush_policy
     */
    void FileRotateWithFilter::flush() {
        logger_->flush();
    }

    std::string FileRotateWithFilter::changeLogFile(const std::string& log_directory) {
        return logger_->changeLogFile(log_directory);
    }

    /// @return the current filename
    std::string FileRotateWithFilter::logFileName() {
        return logger_->logFileName();
    }

    bool FileRotateWithFilter::rotateLog() {
        return logger_->rotateLog();
    }

    void FileRotateWithFilter::setLogSizeCounter() {
        logger_->setLogSizeCounter();
    }

    /**
     * Override the defualt log formatting.
     * Please see https://github.com/KjellKod/g3log/API.markdown for more details
     */
    void FileRotateWithFilter::overrideLogDetails(g3::LogMessage::LogDetailsFunc func) {
        logger_->overrideLogDetails(func);
    }

    void FileRotateWithFilter::overrideLogHeader(const std::string& change) {
        logger_->overrideLogHeader(change);
    }

}// g3 namespace