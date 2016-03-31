#ifndef LSST_QSERV_QDHT_ID_HPP
#define LSST_QSERV_QDHT_ID_HPP

// System headers
#include <algorithm>
#include <array>
#include <cstdint>
#include <iosfwd>
#include <iterator>
#include <stddef.h>
#include <tuple>

namespace lsst {
namespace qserv {
namespace qdht {

class Id
{
public:

    enum class RelPos {
        CW  = -1,
        EQ  =  0,
        CCW =  1
    };

    //----- Default constructor; inits Id to 160 zeroed bits.

    Id();

    //----- Fill this Id with uniformly distributed random bytes.

    void genRandom();

    //----- Convert to/from string representation, meant for human readability.  This will be a 40 character
    //      hex string, with the most-significant bits left-most in the string.

    bool parse(std::string const& str);
    operator std::string() const;

    //----- Convert to/from a sequence of 20 unsigned bytes, meant for efficient binary storage or
    //      transmission. Most significant bytes will be serialized/deserialized first.

    template<typename UInt8OutputIter>
    void serialize(UInt8OutputIter it) const;

    template<typename UInt8InputIter>
    void deserialize(UInt8InputIter it);

    size_t size() const { return sizeof(msb) + sizeof(xsb) + sizeof(lsb); }

    //----- Distance operators between Ids.  diffs() returns a tuple of distances from this to rhs in the ccw
    //      and cw directions around the ring, respectively.  relpos() returns the direction of the shortest
    //      path around the ring from this to rhs, biased to CCW if this and rhs are directly opposite each
    //      other on the ring, or EQ if this and rhs are equal.

    std::tuple<Id, Id> diffs(Id const& rhs) const;
    RelPos relpos(Id const& rhs) const;

    //----- Comparison operators between Ids.  operator< is the non-ring, unsigned, numeric comparison.
    //      modLess() is true whenever rhs is relatively ccw of this (a la relpos() above).

    bool operator==(Id const& rhs) const;
    bool operator<(Id const& rhs) const;
    bool modLess(Id const& rhs) const;

private:

    friend std::ostream& operator<<(std::ostream& os, const Id& id);

    //----- Overloads specialized on the specific iterator traits that we support for serialize and
    //      deserialize.  This way if a client tries to pass an unsupported iterator, they'll get a link
    //      error instead of unexpected behavior at runtime.

    template<typename UInt8OutputIter>
    void serialize(UInt8OutputIter it, uint8_t, std::output_iterator_tag) const;

    template<typename UInt8ForwardIter>
    void serialize(UInt8ForwardIter it, uint8_t, std::forward_iterator_tag) const;

    template<typename UInt8InputIter>
    void deserialize(UInt8InputIter it, uint8_t, std::input_iterator_tag);

    //----- The actual 160 bits of storage, as two 64-bit unsigned ints and a 32-bit unsigned int. This
    //      is done rather than using a single byte array so we can attempt to do 160-bit arithmetic more
    //      efficiently word-by-word instead of byte-by-byte.

    uint64_t msb{0}; // most-significant bits
    uint64_t xsb{0}; // somewhat-significant bits
    uint32_t lsb{0}; // least-significant bits

};

std::ostream& operator<<(std::ostream& os, Id::RelPos const& relpos);
std::ostream& operator<<(std::ostream& os, Id const& id);

}}} // namespace lsst::qserv::qdht

#include <qdht/Id2.cc> // template method defs

#endif // LSST_QSERV_QDHT_ID_HPP
