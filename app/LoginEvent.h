#pragma once

#include <ctb/task/tasks.h>
#include <wx/event.h>

namespace ctb::app
{
   struct LoginEvent final : public wxThreadEvent
   {
      static inline const auto EventType = wxEventTypeTag<LoginEvent>{ wxNewEventType() };

      explicit LoginEvent(tasks::LoginTask::ResultWrapper&& result) :
         wxThreadEvent(EventType),
         m_result(std::move(result))
      {
      }

      LoginEvent(LoginEvent&&) noexcept = default;
      LoginEvent& operator=(LoginEvent&&) noexcept = delete;
      ~LoginEvent() noexcept override = default;

      LoginEvent() = delete;
      LoginEvent(const LoginEvent&) = delete;
      LoginEvent& operator=(const LoginEvent&) = delete;

      tasks::LoginTask::ResultWrapper m_result;
   };

} // namespace ctb::app