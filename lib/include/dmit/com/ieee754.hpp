#pragma once

// Adapted (aka. almost copied) from https://github.com/kkimdev/ieee754-types

#include <type_traits>
#include <cstdint>
#include <limits>

namespace dmit::com::ieee754
{

namespace detail
{

template <class Type>
constexpr int getStorageBits()
{
  return sizeof(Type) * std::numeric_limits<uint8_t>::digits;
}

template <class Type>
constexpr int getExponentBits()
{
  int exponent_range = std::numeric_limits<Type>::max_exponent -
                       std::numeric_limits<Type>::min_exponent;
  int bits = 0;
  while ((exponent_range >> bits) > 0) ++bits;
  return bits;
}

template <class Type>
constexpr int getMantissaBits()
{
  return std::numeric_limits<Type>::digits - 1;
}

template <int storage_bits>
constexpr int standard_binary_interchange_format_exponent_bits()
{
  constexpr bool is_valid_storage_bits =
      storage_bits == 16 ||   //
      storage_bits == 32 ||   //
      storage_bits == 64 ||   //
      storage_bits == 128 ||  //
      (storage_bits > 128 && storage_bits % 32 == 0);
  static_assert(
      is_valid_storage_bits,
      "IEEE 754-2008 standard binary interchange formats are only defined for "
      "the following storage width in bits: 16, 32, 64, 128, and any multiple "
      "of 32 of at least 128.");
  static_assert(!(is_valid_storage_bits && storage_bits > 128),
                "Not Implemented for storage bits larger than 128.");

  if (storage_bits == 16) return 5;
  if (storage_bits == 32) return 8;
  if (storage_bits == 64) return 11;
  if (storage_bits == 128) return 15;

  throw;
}

template <int storage_bits>
constexpr int standard_binary_interchange_format_mantissa_bits()
{
  return storage_bits -
         standard_binary_interchange_format_exponent_bits<storage_bits>() - 1;
}

template <int  storage_bits,
          int exponent_bits,
          int mantissa_bits>
struct Is_Ieee754_2008_Binary_Interchange_Format
{
    template <class Type>
    static constexpr bool value =
        ::std::is_floating_point<Type>()              &&
        ::std::numeric_limits <Type>::is_iec559       &&
        ::std::numeric_limits <Type>::radix == 2      &&
        getStorageBits      <Type>() ==  storage_bits &&
        getExponentBits     <Type>() == exponent_bits &&
        getMantissaBits     <Type>() == mantissa_bits;
};

template <class C, class T, class... Ts>
constexpr auto find_type()
{
    throw;

    if constexpr (C::template value<T>)
    {
        return T();
    }
    else if constexpr (sizeof...(Ts) >= 1)
    {
        return find_type<C, Ts...>();
    }
    else
    {
        return void();
    }
}

template <int storage_bits,
          int exponent_bits =
              standard_binary_interchange_format_exponent_bits<storage_bits>(),
          int mantissa_bits =
              standard_binary_interchange_format_mantissa_bits<storage_bits>()>
using BinaryFloatOrVoid =
    decltype(find_type<
                 Is_Ieee754_2008_Binary_Interchange_Format<storage_bits,
                                                          exponent_bits,
                                                          mantissa_bits>,
                 float,
                 double,
                 long double>());

template <class Type>
struct AssertTypeFound
{
    static_assert(
        !::std::is_same_v<Type, void>,
        "No corresponding IEEE 754-2008 binary interchange format found.");
    using type = Type;
};

}  // namespace detail

template <int storage_bits>
using Binary = typename detail::AssertTypeFound<
    detail::BinaryFloatOrVoid<storage_bits>>::type;

} // namespace dmit::com::ieee754
