#pragma once

#include <chrono>
#include <expected>
#include <future>


namespace ctb::tasks
{
   using namespace std::chrono_literals;


   /// @brief PollingTask - provides a polling interface for an asynchronous task/future
   ///
   template <typename ReturnTypeT>
   class PollingTask
   {
   public:
      using ReturnType     = ReturnTypeT;                      // type that the task function returns
      using FutureType     = std::future<ReturnType>;          // type that std::async(task function...) returns
      using ResultWrapper  = std::expected<ReturnType, Error>; // types that final result is packaged in

      /// @brief Constructor
      /// 
      /// Construct an instance from a std::future
      /// 
      explicit PollingTask(FutureType&& f) noexcept : m_future{ std::move(f) }
      {}

      /// @brief check validity of the task
      /// 
      /// @return true if it contains a valid future that can be waited/retrieved, 
      /// false if future is invalid (uninitialized or already retrieved)
      /// 
      auto isValid() const -> bool { return m_future.valid(); }
     
      /// @brief enum for the current status of the Task
      enum class Status
      {
         Running,
         Deferred,
         Finished,
         Invalid
      };

      /// @brief retrieves the current status of the task.
      /// 
      /// @param timeout - how long the poll will wait for completion before returning
      /// 
      /// @return enum value indicating current status
      /// 
      template<typename RepT, typename PeriodT>
      auto poll(std::chrono::duration<RepT, PeriodT> timeout = 1ms) const noexcept -> Status
      {
         using std::future_status;

         if (!isValid()) 
            return Status::Invalid;

         switch (m_future.wait_for(timeout))
         {
            case future_status::deferred:  return Status::Deferred;
            case future_status::timeout:   return Status::Running;
            case future_status::ready:     return Status::Finished;
            default:                       assert(false);
         }
         return Status::Invalid;
      }

      /// @brief Synchronously retrieve the value for this task.
      /// 
      /// If the task's value isn't available yet because it's still executing, this 
      /// function will block until the value is ready. If you don't want to block,
      /// use poll(). This function never throws: if the task's execution threw an
      /// exception or aborted due to cancellation, an Error will be returned instead
      /// of the expected value.
      /// 
      /// @return the expected value, or an Error if the task threw an exception
      ///
      auto getValue() noexcept -> ResultWrapper
      {
         try 
         {
            return m_future.get();
         }
         catch (...) {
            auto err = packageError();
            log::error("PollingTask::getValue() threw an exception: {}", err.formattedMesage());
            return std::unexpected{ err };
         }
      }

      PollingTask()                              = default;
      PollingTask(PollingTask&&)                 = default;
      PollingTask& operator=(PollingTask&&)      = default;
      ~PollingTask() noexcept                    = default;
      PollingTask(const PollingTask&)            = delete;
      PollingTask& operator=(const PollingTask&) = delete;

   private:
      FutureType m_future{};
   };
 
} //  namespace ctb::tasks
