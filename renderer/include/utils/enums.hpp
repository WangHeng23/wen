#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

template <typename FlagBitsType>
struct FlagTraits
{
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool isBitmask = false;
};

template <typename BitType>
class Flags
{
public:
    using MaskType = typename std::underlying_type<BitType>::type;

    // constructors
    VULKAN_HPP_CONSTEXPR Flags() VULKAN_HPP_NOEXCEPT : m_mask( 0 ) {}

    VULKAN_HPP_CONSTEXPR Flags( BitType bit ) VULKAN_HPP_NOEXCEPT : m_mask( static_cast<MaskType>( bit ) ) {}

    VULKAN_HPP_CONSTEXPR Flags( Flags<BitType> const & rhs ) VULKAN_HPP_NOEXCEPT = default;

    VULKAN_HPP_CONSTEXPR explicit Flags( MaskType flags ) VULKAN_HPP_NOEXCEPT : m_mask( flags ) {}

    // relational operators
#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    auto operator<=>( Flags<BitType> const & ) const = default;
#else
    VULKAN_HPP_CONSTEXPR bool operator<( Flags<BitType> const & rhs ) const VULKAN_HPP_NOEXCEPT
    {
    return m_mask < rhs.m_mask;
    }

    VULKAN_HPP_CONSTEXPR bool operator<=( Flags<BitType> const & rhs ) const VULKAN_HPP_NOEXCEPT
    {
    return m_mask <= rhs.m_mask;
    }

    VULKAN_HPP_CONSTEXPR bool operator>( Flags<BitType> const & rhs ) const VULKAN_HPP_NOEXCEPT
    {
    return m_mask > rhs.m_mask;
    }

    VULKAN_HPP_CONSTEXPR bool operator>=( Flags<BitType> const & rhs ) const VULKAN_HPP_NOEXCEPT
    {
    return m_mask >= rhs.m_mask;
    }

    VULKAN_HPP_CONSTEXPR bool operator==( Flags<BitType> const & rhs ) const VULKAN_HPP_NOEXCEPT
    {
    return m_mask == rhs.m_mask;
    }

    VULKAN_HPP_CONSTEXPR bool operator!=( Flags<BitType> const & rhs ) const VULKAN_HPP_NOEXCEPT
    {
    return m_mask != rhs.m_mask;
    }
#endif

    // logical operator
    VULKAN_HPP_CONSTEXPR bool operator!() const VULKAN_HPP_NOEXCEPT
    {
    return !m_mask;
    }

    // bitwise operators
    VULKAN_HPP_CONSTEXPR Flags<BitType> operator&( Flags<BitType> const & rhs ) const VULKAN_HPP_NOEXCEPT
    {
    return Flags<BitType>( m_mask & rhs.m_mask );
    }

    VULKAN_HPP_CONSTEXPR Flags<BitType> operator|( Flags<BitType> const & rhs ) const VULKAN_HPP_NOEXCEPT
    {
    return Flags<BitType>( m_mask | rhs.m_mask );
    }

    VULKAN_HPP_CONSTEXPR Flags<BitType> operator^( Flags<BitType> const & rhs ) const VULKAN_HPP_NOEXCEPT
    {
    return Flags<BitType>( m_mask ^ rhs.m_mask );
    }

    VULKAN_HPP_CONSTEXPR Flags<BitType> operator~() const VULKAN_HPP_NOEXCEPT
    {
    return Flags<BitType>( m_mask ^ FlagTraits<BitType>::allFlags.m_mask );
    }

    // assignment operators
    VULKAN_HPP_CONSTEXPR_14 Flags<BitType> & operator=( Flags<BitType> const & rhs ) VULKAN_HPP_NOEXCEPT = default;

    VULKAN_HPP_CONSTEXPR_14 Flags<BitType> & operator|=( Flags<BitType> const & rhs ) VULKAN_HPP_NOEXCEPT
    {
    m_mask |= rhs.m_mask;
    return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 Flags<BitType> & operator&=( Flags<BitType> const & rhs ) VULKAN_HPP_NOEXCEPT
    {
    m_mask &= rhs.m_mask;
    return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 Flags<BitType> & operator^=( Flags<BitType> const & rhs ) VULKAN_HPP_NOEXCEPT
    {
    m_mask ^= rhs.m_mask;
    return *this;
    }

    // cast operators
    explicit VULKAN_HPP_CONSTEXPR operator bool() const VULKAN_HPP_NOEXCEPT
    {
    return !!m_mask;
    }

    explicit VULKAN_HPP_CONSTEXPR operator MaskType() const VULKAN_HPP_NOEXCEPT
    {
    return m_mask;
    }

#if defined( VULKAN_HPP_FLAGS_MASK_TYPE_AS_PUBLIC )
public:
#else
private:
#endif
    MaskType m_mask;
};

#if !defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
// relational operators only needed for pre C++20
template <typename BitType>
VULKAN_HPP_CONSTEXPR bool operator<( BitType bit, Flags<BitType> const & flags ) VULKAN_HPP_NOEXCEPT
{
    return flags.operator>( bit );
}

template <typename BitType>
VULKAN_HPP_CONSTEXPR bool operator<=( BitType bit, Flags<BitType> const & flags ) VULKAN_HPP_NOEXCEPT
{
    return flags.operator>=( bit );
}

template <typename BitType>
VULKAN_HPP_CONSTEXPR bool operator>( BitType bit, Flags<BitType> const & flags ) VULKAN_HPP_NOEXCEPT
{
    return flags.operator<( bit );
}

template <typename BitType>
VULKAN_HPP_CONSTEXPR bool operator>=( BitType bit, Flags<BitType> const & flags ) VULKAN_HPP_NOEXCEPT
{
    return flags.operator<=( bit );
}

template <typename BitType>
VULKAN_HPP_CONSTEXPR bool operator==( BitType bit, Flags<BitType> const & flags ) VULKAN_HPP_NOEXCEPT
{
    return flags.operator==( bit );
}

template <typename BitType>
VULKAN_HPP_CONSTEXPR bool operator!=( BitType bit, Flags<BitType> const & flags ) VULKAN_HPP_NOEXCEPT
{
    return flags.operator!=( bit );
}
#endif

// bitwise operators
template <typename BitType>
VULKAN_HPP_CONSTEXPR Flags<BitType> operator&( BitType bit, Flags<BitType> const & flags ) VULKAN_HPP_NOEXCEPT
{
    return flags.operator&( bit );
}

template <typename BitType>
VULKAN_HPP_CONSTEXPR Flags<BitType> operator|( BitType bit, Flags<BitType> const & flags ) VULKAN_HPP_NOEXCEPT
{
    return flags.operator|( bit );
}

template <typename BitType>
VULKAN_HPP_CONSTEXPR Flags<BitType> operator^( BitType bit, Flags<BitType> const & flags ) VULKAN_HPP_NOEXCEPT
{
    return flags.operator^( bit );
}

// bitwise operators on BitType
template <typename BitType, typename std::enable_if<FlagTraits<BitType>::isBitmask, bool>::type = true>
VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR Flags<BitType> operator&( BitType lhs, BitType rhs ) VULKAN_HPP_NOEXCEPT
{
    return Flags<BitType>( lhs ) & rhs;
}

template <typename BitType, typename std::enable_if<FlagTraits<BitType>::isBitmask, bool>::type = true>
VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR Flags<BitType> operator|( BitType lhs, BitType rhs ) VULKAN_HPP_NOEXCEPT
{
    return Flags<BitType>( lhs ) | rhs;
}

template <typename BitType, typename std::enable_if<FlagTraits<BitType>::isBitmask, bool>::type = true>
VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR Flags<BitType> operator^( BitType lhs, BitType rhs ) VULKAN_HPP_NOEXCEPT
{
    return Flags<BitType>( lhs ) ^ rhs;
}

template <typename BitType, typename std::enable_if<FlagTraits<BitType>::isBitmask, bool>::type = true>
VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR Flags<BitType> operator~( BitType bit ) VULKAN_HPP_NOEXCEPT
{
    return ~( Flags<BitType>( bit ) );
}

enum class AttachmentType {
    eColor,
    eDepth,
};

enum class Topology {
    ePointList,
    eLineList,
    eLineStrip,
    eTriangleList,
    eTriangleStrip,
    eTriangleFan,
};

enum class PolygonMode {
    eFill,
    eLine,
    ePoint,
};

enum class CullMode {
    eNone,
    eFront,
    eBack,
    eFrontAndBack,
};

enum class FrontFace {
    eClockwise,
    eCounterClockwise,
};

enum class ShaderStage : uint32_t {
    eVertex = static_cast<uint32_t>(vk::ShaderStageFlagBits::eVertex),
    eFragment = static_cast<uint32_t>(vk::ShaderStageFlagBits::eFragment),
};

using ShaderStages = Flags<ShaderStage>;
template <>
struct FlagTraits<ShaderStage> {
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool isBitmask = true;
    static VULKAN_HPP_CONST_OR_CONSTEXPR ShaderStages allFlags = ShaderStage::eVertex | ShaderStage::eFragment;
};

enum class InputRate {
    eVertex,
    eInstance,
};

enum class VertexType {
    eInt8, eInt8x2, eInt8x3, eInt8x4,
    eInt16, eInt16x2, eInt16x3, eInt16x4,
    eInt32, eInt32x2, eInt32x3, eInt32x4,
    eInt64, eInt64x2, eInt64x3, eInt64x4,
    eUint8, eUint8x2, eUint8x3, eUint8x4,
    eUint16, eUint16x2, eUint16x3, eUint16x4,
    eUint32, eUint32x2, eUint32x3, eUint32x4,
    eUint64, eUint64x2, eUint64x3, eUint64x4,
    eFloat, eFloat2, eFloat3, eFloat4,
    eDouble, eDouble2, eDouble3, eDouble4,
};

enum class IndexType {
    eUint16,
    eUint32,
};

enum class DescriptorType {
    eUniform,
};

} // namespace wen