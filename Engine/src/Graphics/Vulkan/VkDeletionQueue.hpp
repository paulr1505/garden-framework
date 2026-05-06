#pragma once

#include <vector>
#include <functional>
#include <cstdint>
#include <mutex>

class VkDeletionQueue {
public:
    void push(std::function<void()> deleter, uint32_t frameDelay = 3) {
        std::lock_guard<std::mutex> lock(mutex_);
        entries_.push_back({ std::move(deleter), frameDelay });
    }

    void flush() {
        std::vector<std::function<void()>> ready;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            size_t i = 0;
            while (i < entries_.size()) {
                if (entries_[i].frames_remaining == 0) {
                    ready.push_back(std::move(entries_[i].deleter));
                    if (i != entries_.size() - 1)
                        entries_[i] = std::move(entries_.back());
                    entries_.pop_back();
                } else {
                    entries_[i].frames_remaining--;
                    i++;
                }
            }
        }

        for (auto& deleter : ready)
            deleter();
    }

    void flushAll() {
        std::vector<std::function<void()>> ready;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ready.reserve(entries_.size());
            for (auto& entry : entries_)
                ready.push_back(std::move(entry.deleter));
            entries_.clear();
        }

        for (auto& deleter : ready)
            deleter();
    }

private:
    struct DeletionEntry {
        std::function<void()> deleter;
        uint32_t frames_remaining;
    };

    std::vector<DeletionEntry> entries_;
    std::mutex mutex_;
};
