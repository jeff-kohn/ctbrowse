#include "ctwin/CellarTrackerDownload.h"
#include "ctwin/utility.h"

namespace ctwin
{
   CellarTrackerDownload::CellarTrackerDownload(std::string_view user_name, std::string_view pwd)
   {
      setCredentials(user_name, pwd);
   }

   void CellarTrackerDownload::setCredentials(std::string_view user_name, std::string_view pwd)
   {
      m_user = user_name;
      m_pwd = pwd;
   }


}  // namespace ctwin
