///////////////////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 2022 - 2025.
//  Distributed under the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MATHEMATICA_MATHLINK_2022_11_09_H
  #define MATHEMATICA_MATHLINK_2022_11_09_H

  #include <algorithm>
  #include <array>
  #include <cstddef>
  #include <cstdint>
  #include <string>
  #include <type_traits>
  #include <vector>

  extern "C"
  {
    using WSENV                  = struct ml_environment*;
    using WSENVPARAM             = void*;
    using WSEnvironmentParameter = WSENVPARAM;
    using WSEnvironment          = WSENV;
    using WSLINK                 = struct MLink*;

    extern auto WSReleaseString (WSLINK, const char*)      -> void;
    extern auto WSInitialize    (WSEnvironmentParameter)   -> WSEnvironment;
    extern auto WSDeinitialize  (WSEnvironment)            -> void;
    extern auto WSOpen          (int, char**)              -> WSLINK;
    extern auto WSClose         (WSLINK)                   -> int;
    extern auto WSNextPacket    (WSLINK)                   -> int;
    extern auto WSNewPacket     (WSLINK)                   -> int;
    extern auto WSPutFunction   (WSLINK, const char*, int) -> int;
    extern auto WSPutString     (WSLINK, const char*)      -> int;
    extern auto WSEndPacket     (WSLINK)                   -> int;
    extern auto WSError         (WSLINK)                   -> int;
    extern auto WSGetString     (WSLINK, const char**)     -> int;
  }

  namespace mathematica {

  namespace detail {

  // The value of RETURNPKT is 3.
  constexpr int RETURNPKT { INT8_C(3) };

  // Use a local implementation of string copy.
  template<typename DestinationIterator,
           typename SourceIterator>
  constexpr auto strcpy_unsafe(DestinationIterator dst, SourceIterator src) -> DestinationIterator
  {
    while((*dst++ = *src++) != '\0') { ; } // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    return dst;
  }

  constexpr auto strlen_unsafe(const char* p_str) -> ::std::size_t
  {
    ::std::size_t count { ::std::size_t { UINT8_C(0) } };

    while(*p_str != '\0') { ++p_str; ++count; } // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic,altera-id-dependent-backward-branch)

    return count;
  }

  namespace noncopyable_hidden
  {
    struct base_token { };

    class noncopyable : public base_token
    {
    public:
      virtual ~noncopyable() = default;

    protected:
      // Borrow a slightly modified implementation of noncopyable from Boost.
      constexpr noncopyable() = default;

      constexpr noncopyable(const noncopyable&) = delete;

      constexpr noncopyable(noncopyable&&) noexcept = delete;

      constexpr auto operator=(const noncopyable&) -> noncopyable& = delete;

      constexpr auto operator=(noncopyable&&) noexcept -> noncopyable& = delete;
    };
  }

  using noncopyable = noncopyable_hidden::noncopyable;

  } // namespace detail

  class mathematica_mathlink_base : private detail::noncopyable
  {
  public:
    ~mathematica_mathlink_base() noexcept override = default;

    virtual auto send_command(const ::std::string&, ::std::string*) const -> bool = 0;

    auto is_valid() const noexcept -> bool
    {
      return this->get_valid();
    }

  protected:
    constexpr mathematica_mathlink_base() noexcept = default;

  private:
    virtual auto get_valid() const noexcept -> bool = 0;
  };

  template<const char* PtrStrLocationMathLinkKernel = nullptr>
  class mathematica_mathlink : public mathematica_mathlink_base
  {
  public:
    // This is the mathematica_mathlink class.
    // It provides access functions to WSTP.

    // TBD: Make this class thread safe by using synchronization mechanisms
    // when accessing the WSTP-objects.

    mathematica_mathlink() noexcept : my_valid { open() } { }

    ~mathematica_mathlink() noexcept override
    {
      static_cast<void>(close());
    }

    auto send_command(const ::std::string& str_cmd, ::std::string* str_rsp) const -> bool override
    {
      const bool suppress_output { str_rsp == nullptr };

      const ::std::string str_cmd_local { (suppress_output ? str_cmd + ";" : str_cmd) };

      bool
        send_command_is_ok
        {
          (
               put_function("EvaluatePacket", int { INT8_C(1) })
            && put_function("ToExpression", int { INT8_C(1) })
            && put_string(str_cmd_local)
            && end_packet()
          )
        };

      if(send_command_is_ok)
      {
        for(;;)
        {
          // NOTE: This method only works for single returned packets.

          // For instance if the response from the Mathlink kernel has
          // more than one single component, then this sequence fails.

          // Maybe TODO: Extend/expand this to handle multiple returned packets.

          // Skip any (next)-packets before the first ReturnPacket.
          const int next_packet_result { next_packet() };

          if(   (next_packet_result == int { INT8_C(0) })
             || (next_packet_result == return_packet_id()))
          {
            break;
          }

          const int new_packet_result { new_packet() };

          static_cast<void>(new_packet_result);

          if(error() != int { INT8_C(0) })
          {
            send_command_is_ok = false;

            break;
          }
        }
      }

      bool recv_response_is_ok { send_command_is_ok };

      if(send_command_is_ok)
      {
        recv_response_is_ok = (get_string(str_rsp) && recv_response_is_ok);
      }

      return (send_command_is_ok && recv_response_is_ok);
    }

  private:
    const bool my_valid;

    static WSENV  env_ptr;
    static WSLINK lnk_ptr;

    constexpr auto get_valid() const noexcept -> bool override
    {
      return my_valid;
    }

    static constexpr auto return_packet_id() noexcept -> int
    {
      return detail::RETURNPKT;
    }

    static WSENV&  global_env_ptr() noexcept { return env_ptr; }
    static WSLINK& global_lnk_ptr() noexcept { return lnk_ptr; }

    static auto next_packet ()                                   noexcept -> int  { return  ::WSNextPacket (global_lnk_ptr()); }
    static auto new_packet  ()                                   noexcept -> int  { return  ::WSNewPacket  (global_lnk_ptr()); }
    static auto put_function(const ::std::string& str, int argc) noexcept -> bool { return (::WSPutFunction(global_lnk_ptr(), str.c_str(), argc) != 0); }
    static auto put_string  (const ::std::string& str)           noexcept -> bool { return (::WSPutString  (global_lnk_ptr(), str.c_str()) != 0); }
    static auto end_packet  ()                                   noexcept -> bool { return (::WSEndPacket  (global_lnk_ptr()) != 0); }
    static auto error       ()                                   noexcept -> int  { return  ::WSError      (global_lnk_ptr()); }

    static auto get_string(::std::string* str_rsp) -> bool
    {
      const char* p_str_rsp_ws_get { nullptr };

      const bool
        result_get_string_is_ok
        {
          (::WSGetString(global_lnk_ptr(), &p_str_rsp_ws_get) != static_cast<int>(INT8_C(0)))
        };

      if((str_rsp != nullptr) && result_get_string_is_ok)
      {
        const ::std::size_t rsp_len_ws_get { detail::strlen_unsafe(p_str_rsp_ws_get) };

        str_rsp->resize(rsp_len_ws_get);

        static_cast<void>(::std::copy(p_str_rsp_ws_get, p_str_rsp_ws_get + rsp_len_ws_get, str_rsp->begin()));
      }

      ::WSReleaseString(global_lnk_ptr(), p_str_rsp_ws_get);

      return result_get_string_is_ok;
    }

    static auto is_open() noexcept -> bool
    {
      return ((global_env_ptr() != nullptr) && (global_lnk_ptr() != nullptr));
    }

    static auto do_open(const ::std::string& str_location_math_kernel_user) noexcept -> bool
    {
      // Create a list of constant arguments for opening the mathlink kernel.
      using const_args_string_array_type = ::std::array<::std::string, ::std::size_t { UINT8_C(5) }>;

      const const_args_string_array_type
        const_args_strings
        {
          ::std::string("-linkname"),
          str_location_math_kernel_user,
          ::std::string("-linkmode"),
          ::std::string("launch"),
          ::std::string()
        };

      ::std::vector<char> c0(::std::size_t { UINT8_C(1024) }, '\0'); detail::strcpy_unsafe(c0.data(), const_args_strings[static_cast<std::size_t>(UINT8_C(0))].c_str());
      ::std::vector<char> c1(::std::size_t { UINT8_C(4096) }, '\0'); detail::strcpy_unsafe(c1.data(), const_args_strings[static_cast<std::size_t>(UINT8_C(1))].c_str());
      ::std::vector<char> c2(::std::size_t { UINT8_C(1024) }, '\0'); detail::strcpy_unsafe(c2.data(), const_args_strings[static_cast<std::size_t>(UINT8_C(2))].c_str());
      ::std::vector<char> c3(::std::size_t { UINT8_C(1024) }, '\0'); detail::strcpy_unsafe(c3.data(), const_args_strings[static_cast<std::size_t>(UINT8_C(3))].c_str());

      // Create a list of non-constant character pointers for opening the mathlink kernel.
      using nonconst_args_ptrs_array_type = ::std::array<char*, ::std::tuple_size<const_args_string_array_type>::value>;

      auto nonconst_args_pointers =
        nonconst_args_ptrs_array_type
        {
          c0.data(),
          c1.data(),
          c2.data(),
          c3.data(),
          nullptr
        };

      // Open the mathlink kernel.
      global_lnk_ptr() = ::WSOpen(static_cast<int>(const_args_strings.size()), nonconst_args_pointers.data());

      if(global_lnk_ptr() == nullptr)
      {
        ::WSDeinitialize(global_env_ptr());

        global_env_ptr() = nullptr;
      }

      return is_open();
    }

    static auto str_location_mathlink_kernel_default() -> ::std::string
    {
      // TBD: Is it possible (or sensible) to query the Win* registry
      // in order to find the default location of the mathlink kernel?

      // TBD: What about supporting non-Win* platforms like *nix?

      constexpr char
        str_location_math_kernel_default[]
        {
          "\"C:\\Program Files\\Wolfram Research\\Mathematica\\14.0\\MathKernel.exe\""
        };

      return ::std::string { str_location_math_kernel_default };
    }

    static auto open(const char* pstr_location_math_kernel_user = nullptr) noexcept -> bool
    {
      bool result_do_open_is_ok { };

      if(is_open())
      {
        result_do_open_is_ok = true;
      }
      else
      {
        global_env_ptr() = ::WSInitialize(nullptr);

        if(global_env_ptr() != nullptr)
        {
          result_do_open_is_ok =
            do_open
            (
              (pstr_location_math_kernel_user == nullptr) ? str_location_mathlink_kernel_default()
                                                          : ::std::string(pstr_location_math_kernel_user)
            );
        }
      }

      return result_do_open_is_ok;
    }

    static auto close() noexcept -> bool
    {
      const bool result_close_is_ok { is_open() };

      if(global_lnk_ptr() != nullptr)
      {
        static_cast<void>(::WSClose(global_lnk_ptr()));

        global_lnk_ptr() = nullptr;
      }

      if(global_env_ptr() != nullptr)
      {
        ::WSDeinitialize(global_env_ptr());

        global_env_ptr() = nullptr;
      }

      return result_close_is_ok;
    }
  };

  template<const char* PtrStrLocationMathLinkKernel> WSENV  mathematica_mathlink<PtrStrLocationMathLinkKernel>::env_ptr { nullptr };
  template<const char* PtrStrLocationMathLinkKernel> WSLINK mathematica_mathlink<PtrStrLocationMathLinkKernel>::lnk_ptr { nullptr };

  } // namespace mathematica

#endif // MATHEMATICA_MATHLINK_2022_11_09_H
