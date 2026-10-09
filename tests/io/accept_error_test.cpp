#include <iow/io/acceptor/ad_accept_handler.hpp>
#include <fas/testing.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>

UNIT(recoverable_accept_errors, "мягкие ошибки accept должны быть recoverable")
{
  using namespace fas::testing;
  using iow::io::acceptor::is_recoverable_accept_error;
  namespace ae = boost::asio::error;
  namespace se = boost::system::errc;

  t << is_true<assert>(is_recoverable_accept_error(ae::connection_aborted)) << FAS_FL;
  t << is_true<assert>(is_recoverable_accept_error(ae::no_descriptors)) << FAS_FL;
  t << is_true<assert>(is_recoverable_accept_error(ae::no_buffer_space)) << FAS_FL;
  t << is_true<assert>(is_recoverable_accept_error(ae::no_memory)) << FAS_FL;
  t << is_true<assert>(is_recoverable_accept_error(ae::interrupted)) << FAS_FL;
  t << is_true<assert>(is_recoverable_accept_error(
        make_error_code(se::too_many_files_open_in_system))) << FAS_FL;
  t << is_true<assert>(is_recoverable_accept_error(
        make_error_code(se::resource_unavailable_try_again))) << FAS_FL;
}

UNIT(fatal_accept_errors, "жёсткие ошибки accept не recoverable")
{
  using namespace fas::testing;
  using iow::io::acceptor::is_recoverable_accept_error;
  namespace ae = boost::asio::error;

  t << is_false<assert>(is_recoverable_accept_error(ae::bad_descriptor)) << FAS_FL;
  t << is_false<assert>(is_recoverable_accept_error(ae::not_socket)) << FAS_FL;
  t << is_false<assert>(is_recoverable_accept_error(ae::invalid_argument)) << FAS_FL;
  t << is_false<assert>(is_recoverable_accept_error(boost::system::error_code())) << FAS_FL;
}

BEGIN_SUITE(accept_errors, "accept error classification")
  ADD_UNIT(recoverable_accept_errors)
  ADD_UNIT(fatal_accept_errors)
END_SUITE(accept_errors)

BEGIN_TEST
  RUN_SUITE(accept_errors)
END_TEST
