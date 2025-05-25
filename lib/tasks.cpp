#include "ctb/tasks/tasks.h"
#include "ctb/utility.h"
#include "ctb/utility_http.h"

// #include <cpr/cpr.h>


namespace ctb::tasks
{
   using namespace std::literals;
   using cpr::Url;
   using std::exception;
   using std::expected;
   using std::stop_token;
   using std::string_view;
   using std::vector;
   using std::unexpected;


   auto runLoadFileTask(fs::path file, stop_token token) noexcept(false) -> FetchFileTask::ReturnType
   {
      // there was originally more to these functions when implementing a coroutine for libcoro, but
      // when that approach was abandoned for std::async() there wasn't much left. I decided to keep the 
      // encapsulation because at some point I'll probably want to move to asio/cobalt or P2300/?? or ??/??...
      checkStopToken(token);
      return readBinaryFile(file);
   }


   auto runLabelDownloadTask(std::string_view wine_id, std::stop_token token) noexcept(false)-> FetchFileTask::ReturnType
   {
      checkStopToken(token);

      // First, execute task to get the initial HTTP request for the wine page.
      // no need to validate response since it's direct function call and runHttpGetTask
      // already validates it. That might change if this moves to coroutine impl in future
      cpr::Redirect redir{1, true, false, cpr::PostRedirectFlags::POST_301 | cpr::PostRedirectFlags::POST_302};
      auto response = runHttpGetTask(getWineDetailsUrl(wine_id), token, getPageRequestHeaders());
      
      SPDLOG_DEBUG("runLabelDownloadTask({}) successfully got wine details html.", wine_id);

      // parse the HTML to get the URL for the label image.
      checkStopToken(token);
      auto img_url = parseLabelUrlFromHtml(response.text);
      if (img_url.empty())
      {
         throw Error{ constants::ERROR_STR_LABEL_URL_NOT_FOUND };
      }

      SPDLOG_DEBUG("runLabelDownloadTask({}) parsed image url of {}.", wine_id, img_url);

      // now execute task to download the label image from the web
      checkStopToken(token);
      response = runHttpGetTask(img_url, token, getImageRequestHeaders());
      auto [buf, type] = getBytes(response);
      
      SPDLOG_DEBUG("runLabelDownloadTask({}) downloaded {} bytes with content type {}.", wine_id, buf.size(), type);

      return Buffer{ std::from_range, buf };
   }

} // namespace ctb::tasks

 