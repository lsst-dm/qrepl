#ifndef LSST_QSERV_QDHT_ID2_HPP
#define LSST_QSERV_QDHT_ID2_HPP

// Third-party headers
#include "boost/predef/other/endian.h"

namespace lsst {
namespace qserv {
namespace qdht {


template<typename UInt8OutputIter>
void Id::serialize(UInt8OutputIter it) const
{
    typename std::iterator_traits<UInt8OutputIter>::value_type value_type{};
    typename std::iterator_traits<UInt8OutputIter>::iterator_category iterator_category{};
    serialize(it, value_type, iterator_category);
}


template<typename UInt8OutputIter>
void Id::serialize(UInt8OutputIter it, uint8_t, std::output_iterator_tag) const
{
#ifdef BOOST_ENDIAN_LITTLE_BYTE
    uint8_t const* cmsb = reinterpret_cast<uint8_t const*>(&msb);
    for(int i=sizeof(msb); i>=0; --i) { *it++ = cmsb[i]; }
    uint8_t const* cxsb = reinterpret_cast<uint8_t const*>(&xsb);
    for(int i=sizeof(xsb); i>=0; --i) { *it++ = cxsb[i]; }
    uint8_t const* clsb = reinterpret_cast<uint8_t const*>(&lsb);
    for(int i=sizeof(lsb); i>=0; --i) { *it++ = clsb[i]; }
#else
    #error unsupported byte order -- please port Id::serialize() to your machine architecture
#endif
}


//----- All forward iterators are also output iterators, but this is not reflected in the single-inheritance
//      std iterator tag type hierarchy.  So we have to have this separate specialization and re-dispatch.

template<typename UInt8ForwardIter>
void Id::serialize(UInt8ForwardIter it, uint8_t, std::forward_iterator_tag) const
{
    serialize(it, uint8_t{}, std::output_iterator_tag{});
}


template<typename UInt8InputIter>
void Id::deserialize(UInt8InputIter it)
{
    typename std::iterator_traits<UInt8InputIter>::value_type value_type{};
    typename std::iterator_traits<UInt8InputIter>::iterator_category iterator_category{};
    deserialize(it, value_type, iterator_category);
}


template<typename UInt8InputIter>
void Id::deserialize(UInt8InputIter it, uint8_t, std::input_iterator_tag)
{
#ifdef BOOST_ENDIAN_LITTLE_BYTE
    uint8_t* cmsb = reinterpret_cast<uint8_t*>(&msb);
    for(int i=sizeof(msb); i>=0; --i) { cmsb[i] = *it++; }
    uint8_t* cxsb = reinterpret_cast<uint8_t*>(&xsb);
    for(int i=sizeof(xsb); i>=0; --i) { cxsb[i] = *it++; }
    uint8_t* clsb = reinterpret_cast<uint8_t*>(&lsb);
    for(int i=sizeof(lsb); i>=0; --i) { clsb[i] = *it++; }
#else
    #error unsupported byte order -- please port Id::deserialize() to your machine architecture
#endif
}


}}} // namespace lsst::qserv::qdht

#endif // LSST_QSERV_QDHT_ID2_HPP
