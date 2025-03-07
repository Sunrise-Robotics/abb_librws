#include <abb_librws/rws_poco_result.h>

#include <Poco/Net/WebSocket.h>

#include <sstream>


using namespace Poco;
using namespace Poco::Net;


namespace abb :: rws
{
  /***********************************************************************************************************************
   * Struct definitions: POCOResult
   */
  POCOResult::POCOResult(Poco::Net::HTTPResponse::HTTPStatus http_status, std::string const& reason,
    Poco::Net::NameValueCollection const& header_info, std::string const& content)
  : httpStatus_ {http_status}
  , reason_ {reason}
  , headerInfo_(header_info.begin(), header_info.end())
  , content_ {content}
  {
  };

  std::string POCOResult::toString(const bool verbose, const size_t indent) const
  {
    std::stringstream ss;

    std::string seperator = (indent == 0 ? " | " : "\n" + std::string(indent, ' '));

    ss << "HTTP Status: " << static_cast<int>(httpStatus_) << " - " << reason_;

    if (verbose)
    {
      ss << seperator << "HTTP Response Content: " << content_;
    }

    return ss.str();
  }
}