/**
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * This file is part of osm2pgsql (https://osm2pgsql.org/).
 *
 * Copyright (C) 2006-2022 by the osm2pgsql developer community.
 * For a full list of authors see the git log.
 */

#include <catch.hpp>

#include "common-buffer.hpp"

#include "geom-from-osm.hpp"
#include "geom-functions.hpp"
#include "geom.hpp"

#include <array>

TEST_CASE("geom::linestring_t", "[NoDB]")
{
    geom::linestring_t ls1;

    REQUIRE(ls1.empty());
    ls1.emplace_back(17, 42);
    ls1.emplace_back(-3, 22);
    REQUIRE(ls1.size() == 2);

    auto it = ls1.cbegin();
    REQUIRE(it != ls1.cend());
    REQUIRE(it->x() == 17);
    ++it;
    REQUIRE(it != ls1.cend());
    REQUIRE(it->y() == 22);
    ++it;
    REQUIRE(it == ls1.cend());

    REQUIRE(ls1.num_geometries() == 1);
}

TEST_CASE("line geometry", "[NoDB]")
{
    geom::geometry_t const geom{geom::linestring_t{{1, 1}, {2, 2}}};

    REQUIRE(num_geometries(geom) == 1);
    REQUIRE(area(geom) == Approx(0.0));
    REQUIRE(geometry_type(geom) == "LINESTRING");
    REQUIRE(centroid(geom) == geom::geometry_t{geom::point_t{1.5, 1.5}});
}

TEST_CASE("create_linestring from OSM data", "[NoDB]")
{
    test_buffer_t buffer;
    buffer.add_node("w20 Nn1x1y1,n2x2y2");

    auto const geom =
        geom::create_linestring(buffer.buffer().get<osmium::Way>(0));

    REQUIRE(geom.is_linestring());
    REQUIRE(geometry_type(geom) == "LINESTRING");
    REQUIRE(num_geometries(geom) == 1);
    REQUIRE(area(geom) == Approx(0.0));
    REQUIRE(geom.get<geom::linestring_t>() ==
            geom::linestring_t{{1, 1}, {2, 2}});
    REQUIRE(centroid(geom) == geom::geometry_t{geom::point_t{1.5, 1.5}});
}

TEST_CASE("create_linestring from OSM data without locations", "[NoDB]")
{
    test_buffer_t buffer;
    buffer.add_node("w20 Nn1,n2");

    auto const geom =
        geom::create_linestring(buffer.buffer().get<osmium::Way>(0));

    REQUIRE(geom.is_null());
}

TEST_CASE("create_linestring from invalid OSM data", "[NoDB]")
{
    test_buffer_t buffer;
    buffer.add_node("w20 Nn1x1y1");

    auto const geom =
        geom::create_linestring(buffer.buffer().get<osmium::Way>(0));

    REQUIRE(geom.is_null());
}

TEST_CASE("geom::segmentize w/o split", "[NoDB]")
{
    geom::linestring_t const line{{0, 0}, {1, 2}, {2, 2}};

    auto const geom = geom::segmentize(geom::geometry_t{line}, 10.0);

    REQUIRE(geom.is_multilinestring());
    REQUIRE(num_geometries(geom) == 1);
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 1);
    REQUIRE(ml[0] == line);
}

TEST_CASE("geom::segmentize with split 0.5", "[NoDB]")
{
    geom::linestring_t const line{{0, 0}, {1, 0}};

    std::array<geom::linestring_t, 2> const expected{
        geom::linestring_t{{0, 0}, {0.5, 0}},
        geom::linestring_t{{0.5, 0}, {1, 0}}};

    auto const geom = geom::segmentize(geom::geometry_t{line}, 0.5);

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 2);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
}

TEST_CASE("geom::segmentize with split 0.4", "[NoDB]")
{
    geom::linestring_t const line{{0, 0}, {1, 0}};

    std::array<geom::linestring_t, 3> const expected{
        geom::linestring_t{{0, 0}, {0.4, 0}},
        geom::linestring_t{{0.4, 0}, {0.8, 0}},
        geom::linestring_t{{0.8, 0}, {1, 0}}};

    auto const geom = geom::segmentize(geom::geometry_t{line}, 0.4);

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 3);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
    REQUIRE(ml[2] == expected[2]);
}

TEST_CASE("geom::segmentize with split 1.0 at start", "[NoDB]")
{
    geom::linestring_t const line{{0, 0}, {2, 0}, {3, 0}, {4, 0}};

    std::array<geom::linestring_t, 4> const expected{
        geom::linestring_t{{0, 0}, {1, 0}}, geom::linestring_t{{1, 0}, {2, 0}},
        geom::linestring_t{{2, 0}, {3, 0}}, geom::linestring_t{{3, 0}, {4, 0}}};

    auto const geom = geom::segmentize(geom::geometry_t{line}, 1.0);

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 4);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
    REQUIRE(ml[2] == expected[2]);
    REQUIRE(ml[3] == expected[3]);
}

TEST_CASE("geom::segmentize with split 1.0 in middle", "[NoDB]")
{
    geom::linestring_t const line{{0, 0}, {1, 0}, {3, 0}, {4, 0}};

    std::array<geom::linestring_t, 4> const expected{
        geom::linestring_t{{0, 0}, {1, 0}}, geom::linestring_t{{1, 0}, {2, 0}},
        geom::linestring_t{{2, 0}, {3, 0}}, geom::linestring_t{{3, 0}, {4, 0}}};

    auto const geom = geom::segmentize(geom::geometry_t{line}, 1.0);

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 4);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
    REQUIRE(ml[2] == expected[2]);
    REQUIRE(ml[3] == expected[3]);
}

TEST_CASE("geom::segmentize with split 1.0 at end", "[NoDB]")
{
    geom::linestring_t const line{{0, 0}, {1, 0}, {2, 0}, {4, 0}};

    std::array<geom::linestring_t, 4> const expected{
        geom::linestring_t{{0, 0}, {1, 0}}, geom::linestring_t{{1, 0}, {2, 0}},
        geom::linestring_t{{2, 0}, {3, 0}}, geom::linestring_t{{3, 0}, {4, 0}}};

    auto const geom = geom::segmentize(geom::geometry_t{line}, 1.0);

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 4);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
    REQUIRE(ml[2] == expected[2]);
    REQUIRE(ml[3] == expected[3]);
}

TEST_CASE("create_multilinestring with single line", "[NoDB]")
{
    geom::linestring_t const expected{{1, 1}, {2, 1}};

    test_buffer_t buffer;
    buffer.add_way("w20 Nn10x1y1,n11x2y1");

    auto const geom =
        geom::line_merge(geom::create_multilinestring(buffer.buffer()));

    REQUIRE(geom.is_multilinestring());
    REQUIRE(geometry_type(geom) == "MULTILINESTRING");
    REQUIRE(num_geometries(geom) == 1);
    REQUIRE(area(geom) == Approx(0.0));
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 1);
    REQUIRE(ml[0] == expected);
}

TEST_CASE("create_multilinestring with single line forming a ring", "[NoDB]")
{
    geom::linestring_t const expected{{1, 1}, {2, 1}, {2, 2}, {1, 1}};

    test_buffer_t buffer;
    buffer.add_way("w20 Nn10x1y1,n11x2y1,n12x2y2,n10x1y1");

    auto const geom =
        geom::line_merge(geom::create_multilinestring(buffer.buffer()));

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 1);
    REQUIRE(ml[0] == expected);
}

TEST_CASE("create_multilinestring from two non-joined lines", "[NoDB]")
{
    std::array<geom::linestring_t, 2> const expected{
        geom::linestring_t{{1, 1}, {2, 1}}, geom::linestring_t{{2, 2}, {3, 2}}};

    test_buffer_t buffer;
    buffer.add_way("w20 Nn10x1y1,n11x2y1");
    buffer.add_way("w21 Nn12x2y2,n13x3y2");

    auto const geom =
        geom::line_merge(geom::create_multilinestring(buffer.buffer()));

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 2);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
}

TEST_CASE("create_multilinestring from two lines end to end", "[NoDB]")
{
    geom::linestring_t const expected{{1, 1}, {2, 1}, {2, 2}};

    test_buffer_t buffer;
    buffer.add_way("w20 Nn10x1y1,n11x2y1");
    buffer.add_way("w21 Nn11x2y1,n12x2y2");

    auto const geom =
        geom::line_merge(geom::create_multilinestring(buffer.buffer()));

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 1);
    REQUIRE(ml[0] == expected);
}

TEST_CASE("create_multilinestring from two lines with same start point",
          "[NoDB]")
{
    geom::linestring_t const expected{{2, 1}, {1, 1}, {1, 2}};

    test_buffer_t buffer;
    buffer.add_way("w20 Nn10x1y1,n11x2y1");
    buffer.add_way("w21 Nn10x1y1,n12x1y2");

    auto const geom =
        geom::line_merge(geom::create_multilinestring(buffer.buffer()));

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 1);
    REQUIRE(ml[0] == expected);
}

TEST_CASE("create_multilinestring from two lines with same end point", "[NoDB]")
{
    geom::linestring_t const expected{{1, 2}, {1, 1}, {2, 1}};

    test_buffer_t buffer;
    buffer.add_way("w20 Nn10x1y2,n11x1y1");
    buffer.add_way("w21 Nn12x2y1,n11x1y1");

    auto const geom =
        geom::line_merge(geom::create_multilinestring(buffer.buffer()));

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 1);
    REQUIRE(ml[0] == expected);
}

TEST_CASE("create_multilinestring from two lines connected end to end forming "
          "a ring",
          "[NoDB]")
{
    geom::linestring_t const expected{{1, 1}, {2, 1}, {2, 2}, {1, 2}, {1, 1}};

    test_buffer_t buffer;
    buffer.add_way("w20 Nn10x1y1,n11x2y1,n13x2y2");
    buffer.add_way("w21 Nn13x2y2,n12x1y2,n10x1y1");

    auto const geom =
        geom::line_merge(geom::create_multilinestring(buffer.buffer()));

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 1);
    REQUIRE(ml[0] == expected);
}

TEST_CASE("create_multilinestring from two lines with same start and end point",
          "[NoDB]")
{
    geom::linestring_t const expected{{2, 2}, {2, 1}, {1, 1}, {1, 2}, {2, 2}};

    test_buffer_t buffer;
    buffer.add_way("w20 Nn10x1y1,n11x2y1,n13x2y2");
    buffer.add_way("w21 Nn10x1y1,n12x1y2,n13x2y2");

    auto const geom =
        geom::line_merge(geom::create_multilinestring(buffer.buffer()));

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 1);
    REQUIRE(ml[0] == expected);
}

TEST_CASE("create_multilinestring from three lines, two with same start and "
          "end point",
          "[NoDB]")
{
    geom::linestring_t const expected{{2, 2}, {2, 1}, {1, 1}, {1, 2}, {2, 2}};

    test_buffer_t buffer;
    buffer.add_way("w20 Nn10x1y1,n11x2y1,n13x2y2");
    buffer.add_way("w21 Nn10x1y1,n12x1y2");
    buffer.add_way("w22 Nn12x1y2,n13x2y2");

    auto const geom =
        geom::line_merge(geom::create_multilinestring(buffer.buffer()));

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 1);
    REQUIRE(ml[0] == expected);
}

TEST_CASE("create_multilinestring from four lines forming two rings", "[NoDB]")
{
    std::array<geom::linestring_t, 2> const expected{
        geom::linestring_t{{2, 1}, {1, 1}, {1, 2}},
        geom::linestring_t{{3, 4}, {3, 3}, {4, 3}}};

    test_buffer_t buffer;
    buffer.add_way("w20 Nn10x1y1,n11x2y1");
    buffer.add_way("w21 Nn10x1y1,n12x1y2");
    buffer.add_way("w22 Nn13x3y4,n14x3y3");
    buffer.add_way("w23 Nn15x4y3,n14x3y3");

    auto const geom =
        geom::line_merge(geom::create_multilinestring(buffer.buffer()));

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 2);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
}

TEST_CASE("create_multilinestring from Y shape", "[NoDB]")
{
    std::array<geom::linestring_t, 2> const expected{
        geom::linestring_t{{2, 1}, {1, 1}, {1, 2}}, geom::linestring_t{
                                                        {1, 1},
                                                        {2, 2},
                                                    }};

    test_buffer_t buffer;
    buffer.add_way("w20 Nn10x1y1,n11x2y1");
    buffer.add_way("w21 Nn10x1y1,n12x1y2");
    buffer.add_way("w22 Nn10x1y1,n13x2y2");

    auto const geom =
        geom::line_merge(geom::create_multilinestring(buffer.buffer()));

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 2);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
}

TEST_CASE("create_multilinestring from P shape", "[NoDB]")
{
    geom::linestring_t const expected{{1, 1}, {1, 2}, {1, 3}, {2, 3}, {1, 2}};

    test_buffer_t buffer;
    buffer.add_way("w20 Nn10x1y1,n11x1y2,n12x1y3");
    buffer.add_way("w21 Nn12x1y3,n13x2y3,n11x1y2");

    auto const geom =
        geom::line_merge(geom::create_multilinestring(buffer.buffer()));

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 1);
    REQUIRE(ml[0] == expected);
}

TEST_CASE("create_multilinestring from P shape with closed way", "[NoDB]")
{
    std::array<geom::linestring_t, 2> const expected{
        geom::linestring_t{{1, 2}, {1, 1}}, geom::linestring_t{
                                                {1, 2},
                                                {1, 3},
                                                {2, 3},
                                                {1, 2},
                                            }};

    test_buffer_t buffer;
    buffer.add_way("w20 Nn11x1y2,n12x1y3,n13x2y3,n11x1y2");
    buffer.add_way("w21 Nn11x1y2,n10x1y1");

    auto const geom =
        geom::line_merge(geom::create_multilinestring(buffer.buffer()));

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 2);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
}
