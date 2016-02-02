// Class header
#include "Id.hpp"

// System headers
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <tuple>

using namespace std;

namespace lsst {
namespace qserv {
namespace qdht {


Id::Id()
{
}


void Id::genRandom()
{
    static random_device e;
    static uniform_int_distribution<uint64_t> dq(0, UINT64_MAX);
    static uniform_int_distribution<uint32_t> dl(0, UINT32_MAX);
    msb = dq(e);
    xsb = dq(e);
    lsb = dl(e);
}


bool Id::parse(string const& str)
{
    return (str.size() == 40)
        && (sscanf(&str[0], "%16lx", &msb) == 1)
        && (sscanf(&str[16], "%16lx", &xsb) == 1)
        && (sscanf(&str[32], "%8x", &lsb) == 1);
}


Id::operator string() const
{
    stringstream str;
    str << *this;
    return str.str();
}


bool Id::operator==(Id const& rhs) const
{
    return (msb == rhs.msb) && (xsb == rhs.xsb) && (lsb == rhs.lsb);
}


bool Id::operator<(Id const& rhs) const
{
    if (msb == rhs.msb) {
        if (xsb == rhs.xsb) {
            return (lsb < rhs.lsb);
        } else {
            return (xsb < rhs.xsb);
        }
    } else {
        return (msb < rhs.msb);
    }
}


bool Id::modLess(Id const& rhs) const
{
    return relpos(rhs) == RelPos::CCW;
}


tuple<Id, Id> Id::diffs(Id const& rhs) const
{
    Id ccw, cw;             // differences -- counter-clockwise and clockwise
    uint32_t b, c;          // borrow and carry
    if (*this < rhs) {

        ccw.lsb = rhs.lsb - lsb;
        b = (ccw.lsb > rhs.lsb) ? 1 : 0;
        ccw.xsb = rhs.xsb - (xsb + b);
        b = ((ccw.xsb > rhs.xsb) | (b && (ccw.xsb == rhs.xsb))) ? 1 : 0;
        ccw.msb = rhs.msb - (msb + b);

        cw.lsb = UINT32_MAX - ccw.lsb + 1;
        c = (cw.lsb == 0) ? 1 : 0;
        cw.xsb = UINT64_MAX - ccw.xsb + c;
        c = (c && (cw.xsb == 0)) ? 1 : 0;
        cw.msb = UINT64_MAX - ccw.msb + c;

    } else {

        cw.lsb = lsb - rhs.lsb;
        b = (cw.lsb > lsb) ? 1 : 0;
        cw.xsb = xsb - (rhs.xsb + b);
        b = ((cw.xsb > xsb) | (b && (cw.xsb == xsb))) ? 1 : 0;
        cw.msb = msb - (rhs.msb + b);

        ccw.lsb = UINT32_MAX - cw.lsb + 1;
        c = (ccw.lsb == 0) ? 1 : 0;
        ccw.xsb = UINT64_MAX - cw.xsb + c;
        c = (c && (ccw.xsb == 0)) ? 1 : 0;
        ccw.msb = UINT64_MAX - cw.msb + c;

    }
    return make_tuple(ccw, cw);
}


Id::RelPos Id::relpos(Id const& rhs) const
{
    if (*this == rhs) {
        return RelPos::EQ;
    } else {
        Id ccw, cw;
        tie(ccw, cw) = diffs(rhs);
        return (cw < ccw) ? RelPos::CW : RelPos::CCW;
    }
}


ostream& operator<<(ostream& os, Id const& id)
{
    std::ios state(NULL);
    state.copyfmt(os);
    os << hex << uppercase << setfill('0');
    os << setw(16) << id.msb << setw(16) << id.xsb << setw(8) << id.lsb;
    os.copyfmt(state);
    return os;
}


ostream& operator<<(ostream& os, Id::RelPos const& relpos)
{
    switch(relpos) {

    case Id::RelPos::CW:
        os << "CW";
        break;

    case Id::RelPos::EQ:
        os << "EQ";
        break;

    case Id::RelPos::CCW:
        os << "CCW";
        break;

    default:
        os << "RelPos( " << static_cast<int>(relpos) << ")";
        break;

    }

    return os;
}


}}} // namespace lsst::qserv::qdht
