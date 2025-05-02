#pragma once

#include <ctb/task/tasks.h>
#include <wx/event.h>

namespace ctb::app
{
   struct wx_LoginEvent final : public wxThreadEvent
   {
      static inline const auto EventType = wxEventTypeTag<wx_LoginEvent>{ wxNewEventType() };

      explicit wx_LoginEvent(tasks::LoginTask::ResultWrapper&& result) :
         wxThreadEvent(EventType),
         m_result(std::move(result))
      {
      }

      wx_LoginEvent(wx_LoginEvent&&) noexcept = default;
      wx_LoginEvent& operator=(wx_LoginEvent&&) noexcept = default;
      ~wx_LoginEvent() noexcept override = default;

      wx_LoginEvent() = delete;
      wx_LoginEvent(const wx_LoginEvent&) = delete;
      wx_LoginEvent& operator=(const wx_LoginEvent&) = delete;

      tasks::LoginTask::ResultWrapper m_result;
   };

} // namespace ctb::app