#define BOOST_TEST_MODULE qdht
#include "boost/test/included/unit_test.hpp"

#include "qdht/Id.hpp"

using namespace std;

namespace lsst {
namespace qserv {
namespace qdht {


BOOST_AUTO_TEST_CASE(id_default_ctor)
{
    qdht::Id a;
    BOOST_TEST(string(a) == "0000000000000000000000000000000000000000");
}


BOOST_AUTO_TEST_CASE(id_string)
{
    qdht::Id a;
    a.parse("00112233445566778899AABBCCDDEEFF12345678");
    BOOST_TEST(string(a) == "00112233445566778899AABBCCDDEEFF12345678");
}


BOOST_AUTO_TEST_CASE(id_equal)
{
    qdht::Id a, b, c, d;

    a.parse("0000000000000000000000000000000000000000");
    b.parse("00000000000000000000000000000000FFFFFFFF");
    c.parse("0000000000000000FFFFFFFFFFFFFFFFFFFFFFFF");
    d.parse("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

    BOOST_TEST(a == a);
    BOOST_TEST(!(a == b));
    BOOST_TEST(!(a == c));
    BOOST_TEST(!(a == d));

    BOOST_TEST(!(b == a));
    BOOST_TEST(b == b);
    BOOST_TEST(!(b == c));
    BOOST_TEST(!(b == d));

    BOOST_TEST(!(c == a));
    BOOST_TEST(!(c == b));
    BOOST_TEST(c == c);
    BOOST_TEST(!(c == d));

    BOOST_TEST(!(d == a));
    BOOST_TEST(!(d == b));
    BOOST_TEST(!(d == c));
    BOOST_TEST(d == d);
}


BOOST_AUTO_TEST_CASE(id_less_than)
{
    qdht::Id a, b, c, d;

    a.parse("3333333333333333444444444444444455555555");
    b.parse("3333333333333333444444444444444466666666");
    c.parse("3333333333333333555555555555555566666666");
    d.parse("4444444444444444555555555555555566666666");

    BOOST_TEST(!(a<a));
    BOOST_TEST(a<b);
    BOOST_TEST(a<c);
    BOOST_TEST(a<d);

    BOOST_TEST(!(b<a));
    BOOST_TEST(!(b<b));
    BOOST_TEST(b<c);
    BOOST_TEST(b<d);

    BOOST_TEST(!(c<a));
    BOOST_TEST(!(c<b));
    BOOST_TEST(!(c<c));
    BOOST_TEST(c<d);

    BOOST_TEST(!(d<a));
    BOOST_TEST(!(d<b));
    BOOST_TEST(!(d<c));
    BOOST_TEST(!(d<d));
}


BOOST_AUTO_TEST_CASE(id_diffs)
{
    qdht::Id a, b, ccw, cw;

    // test borrow through Fs

    a.parse("7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    b.parse("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE");

    tie(ccw, cw) = a.diffs(b);
    BOOST_TEST(string(ccw) == "7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    BOOST_TEST(string(cw)  == "8000000000000000000000000000000000000001");

    tie(ccw, cw) = b.diffs(a);
    BOOST_TEST(string(ccw) == "8000000000000000000000000000000000000001");
    BOOST_TEST(string(cw)  == "7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

    // test borrow through 0s

    a.parse("0000000000000000000000000000000000000001");
    b.parse("8000000000000000000000000000000000000000");

    tie(ccw, cw) = a.diffs(b);
    BOOST_TEST(string(ccw) == "7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    BOOST_TEST(string(cw)  == "8000000000000000000000000000000000000001");

    tie(ccw, cw) = b.diffs(a);
    BOOST_TEST(string(ccw) == "8000000000000000000000000000000000000001");
    BOOST_TEST(string(cw)  == "7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
}


BOOST_AUTO_TEST_CASE(id_relpos)
{
    qdht::Id a, b, c, d, e, f;

    a.parse("0000000000000000000000000000000000000000");  // relpos test cases
    b.parse("0000000000000000000000000000000000000001");  //
    c.parse("7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");  //    c:7F    b:01
    d.parse("8000000000000000000000000000000000000000");  //   d:80      a:00
    e.parse("8000000000000000000000000000000000000001");  //    e:81    f:FF
    f.parse("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");  //

    BOOST_TEST(a.relpos(a) == qdht::Id::RelPos::EQ);
    BOOST_TEST(a.relpos(b) == qdht::Id::RelPos::CCW);
    BOOST_TEST(a.relpos(c) == qdht::Id::RelPos::CCW);
    BOOST_TEST(a.relpos(d) == qdht::Id::RelPos::CCW);
    BOOST_TEST(a.relpos(e) == qdht::Id::RelPos::CW);
    BOOST_TEST(a.relpos(f) == qdht::Id::RelPos::CW);

    BOOST_TEST(b.relpos(a) == qdht::Id::RelPos::CW);
    BOOST_TEST(b.relpos(b) == qdht::Id::RelPos::EQ);
    BOOST_TEST(b.relpos(c) == qdht::Id::RelPos::CCW);
    BOOST_TEST(b.relpos(d) == qdht::Id::RelPos::CCW);
    BOOST_TEST(b.relpos(e) == qdht::Id::RelPos::CCW);
    BOOST_TEST(b.relpos(f) == qdht::Id::RelPos::CW);

    BOOST_TEST(c.relpos(a) == qdht::Id::RelPos::CW);
    BOOST_TEST(c.relpos(b) == qdht::Id::RelPos::CW);
    BOOST_TEST(c.relpos(c) == qdht::Id::RelPos::EQ);
    BOOST_TEST(c.relpos(d) == qdht::Id::RelPos::CCW);
    BOOST_TEST(c.relpos(e) == qdht::Id::RelPos::CCW);
    BOOST_TEST(c.relpos(f) == qdht::Id::RelPos::CCW);

    BOOST_TEST(d.relpos(a) == qdht::Id::RelPos::CCW);
    BOOST_TEST(d.relpos(b) == qdht::Id::RelPos::CW);
    BOOST_TEST(d.relpos(c) == qdht::Id::RelPos::CW);
    BOOST_TEST(d.relpos(d) == qdht::Id::RelPos::EQ);
    BOOST_TEST(d.relpos(e) == qdht::Id::RelPos::CCW);
    BOOST_TEST(d.relpos(f) == qdht::Id::RelPos::CCW);

    BOOST_TEST(e.relpos(a) == qdht::Id::RelPos::CCW);
    BOOST_TEST(e.relpos(b) == qdht::Id::RelPos::CCW);
    BOOST_TEST(e.relpos(c) == qdht::Id::RelPos::CW);
    BOOST_TEST(e.relpos(d) == qdht::Id::RelPos::CW);
    BOOST_TEST(e.relpos(e) == qdht::Id::RelPos::EQ);
    BOOST_TEST(e.relpos(f) == qdht::Id::RelPos::CCW);

    BOOST_TEST(f.relpos(a) == qdht::Id::RelPos::CCW);
    BOOST_TEST(f.relpos(b) == qdht::Id::RelPos::CCW);
    BOOST_TEST(f.relpos(c) == qdht::Id::RelPos::CCW);
    BOOST_TEST(f.relpos(d) == qdht::Id::RelPos::CW);
    BOOST_TEST(f.relpos(e) == qdht::Id::RelPos::CW);
    BOOST_TEST(f.relpos(f) == qdht::Id::RelPos::EQ);
}


BOOST_AUTO_TEST_CASE(id_modless)
{
    qdht::Id a, b, c, d, e, f;

    a.parse("0000000000000000000000000000000000000000");  // modless test cases
    b.parse("0000000000000000000000000000000000000001");  //
    c.parse("7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");  //    c:7F    b:01
    d.parse("8000000000000000000000000000000000000000");  //   d:80      a:00
    e.parse("8000000000000000000000000000000000000001");  //    e:81    f:FF
    f.parse("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");  //

    BOOST_TEST(!a.modLess(a));
    BOOST_TEST(a.modLess(b));
    BOOST_TEST(a.modLess(c));
    BOOST_TEST(a.modLess(d));
    BOOST_TEST(!a.modLess(e));
    BOOST_TEST(!a.modLess(f));

    BOOST_TEST(!b.modLess(a));
    BOOST_TEST(!b.modLess(b));
    BOOST_TEST(b.modLess(c));
    BOOST_TEST(b.modLess(d));
    BOOST_TEST(b.modLess(e));
    BOOST_TEST(!b.modLess(f));

    BOOST_TEST(!c.modLess(a));
    BOOST_TEST(!c.modLess(b));
    BOOST_TEST(!c.modLess(c));
    BOOST_TEST(c.modLess(d));
    BOOST_TEST(c.modLess(e));
    BOOST_TEST(c.modLess(f));

    BOOST_TEST(d.modLess(a));
    BOOST_TEST(!d.modLess(b));
    BOOST_TEST(!d.modLess(c));
    BOOST_TEST(!d.modLess(d));
    BOOST_TEST(d.modLess(e));
    BOOST_TEST(d.modLess(f));

    BOOST_TEST(e.modLess(a));
    BOOST_TEST(e.modLess(b));
    BOOST_TEST(!e.modLess(c));
    BOOST_TEST(!e.modLess(d));
    BOOST_TEST(!e.modLess(e));
    BOOST_TEST(e.modLess(f));

    BOOST_TEST(f.modLess(a));
    BOOST_TEST(f.modLess(b));
    BOOST_TEST(f.modLess(c));
    BOOST_TEST(!f.modLess(d));
    BOOST_TEST(!f.modLess(e));
    BOOST_TEST(!f.modLess(f));
}


BOOST_AUTO_TEST_CASE(id_serdes)
{
    qdht::Id a, b;
    a.parse("00112233445566778899AABBCCDDEEFF12345678");
    uint8_t cereal[20];
    a.serialize(cereal);
    b.deserialize(cereal);
    BOOST_TEST(string(b) == "00112233445566778899AABBCCDDEEFF12345678");
}


}}} // namespace lsst::qserv::qdht
