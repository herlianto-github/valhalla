#include "gurka.h"
#include <gtest/gtest.h>

#if !defined(VALHALLA_SOURCE_DIR)
#define VALHALLA_SOURCE_DIR
#endif

using namespace valhalla;

class InstructionsNamedRoundabout : public ::testing::Test {
protected:
  static gurka::map map;

  static void SetUpTestSuite() {
    constexpr double gridsize = 100;

    const std::string ascii_map = R"(
            I
            |
            H
           / \
       A--B   E--F-G
           \ /
            C
            |
            D
    )";

    const gurka::ways ways = {{"AB",
                               {{"highway", "primary"},
                                {"name", "East Governor Road"},
                                {"ref", "A 1"},
                                {"direction", "East"},
                                {"destination", "München;Kürten;Dhünn"},
                                {"destination:ref", "A 95;B 2;B 300"}}},
                              {"BCEHB",
                               {{"highway", "primary"},
                                {"oneway", "yes"},
                                {"junction", "roundabout"},
                                {"name", "Dupont Circle"}}},
                              {"CD", {{"highway", "primary"}, {"name", "Homestead Lane"}}},
                              {"EF",
                               {{"highway", "primary"},
                                {"name", "East Governor Road"},
                                {"ref", "A 1"},
                                {"direction", "East"}}},
                              {"FG", {{"highway", "primary"}, {"ref", "A 1"}, {"direction", "East"}}},
                              {"HI", {{"highway", "primary"}, {"name", ""}}}};

    const auto layout = gurka::detail::map_to_coordinates(ascii_map, 100, {5.1079374, 52.0887174});
    map = gurka::buildtiles(layout, ways, {}, {}, "test/data/gurka_named_roundabouts",
                            {{"mjolnir.data_processing.use_direction_on_ways", "true"},
                             {"mjolnir.admin",
                              {VALHALLA_SOURCE_DIR "test/data/netherlands_admin.sqlite"}}});
  }
};

gurka::map InstructionsNamedRoundabout::map = {};

/*************************************************************/

// enter_roundabout_verbal
// "8": "Enter <STREET_NAMES>"
TEST_F(InstructionsNamedRoundabout, RoundaboutEnterOnly) {
  auto result = gurka::route(map, "A", "E", "auto");
  gurka::assert::raw::expect_maneuvers(result, {DirectionsLeg_Maneuver_Type_kStart,
                                                DirectionsLeg_Maneuver_Type_kRoundaboutEnter,
                                                DirectionsLeg_Maneuver_Type_kDestination});
  int maneuver_index = 1;

  // TODO: known issue - future update to end on a roundabout
  // Verify the enter_roundabout instructions
  //  gurka::assert::raw::expect_instructions_at_maneuver_index(result, maneuver_index,
  //                                                            "Enter Dupont Circle.",
  //                                                            "Enter Dupont Circle.",
  //                                                            "Enter Dupont Circle.", "");
}

// enter_roundabout_verbal
// 9": "Enter <STREET_NAMES> and take the <ORDINAL_VALUE> exit."
TEST_F(InstructionsNamedRoundabout, RoundaboutOrdinalOnly) {
  auto result = gurka::route(map, "A", "I", "auto");
  gurka::assert::raw::expect_maneuvers(result, {DirectionsLeg_Maneuver_Type_kStart,
                                                DirectionsLeg_Maneuver_Type_kRoundaboutEnter,
                                                DirectionsLeg_Maneuver_Type_kRoundaboutExit,
                                                DirectionsLeg_Maneuver_Type_kDestination});
  int maneuver_index = 1;

  // Verify the enter_roundabout instructions
  gurka::assert::raw::
      expect_instructions_at_maneuver_index(result, maneuver_index,
                                            "Enter Dupont Circle and take the 3rd exit.",
                                            "Enter Dupont Circle and take the 3rd exit.",
                                            "Enter Dupont Circle and take the 3rd exit.", "");

  // Verify the exit_roundabout instructions
  gurka::assert::raw::expect_instructions_at_maneuver_index(
      result, ++maneuver_index, "Exit the roundabout.", "",
      "Exit the roundabout. Then You will arrive at your destination.", "Continue for 200 meters.");
}

// enter_roundabout_verbal
// "10": "Enter <STREET_NAMES> and take the <ORDINAL_VALUE> exit onto <ROUNDABOUT_EXIT_STREET_NAMES>."
TEST_F(InstructionsNamedRoundabout, RoundaboutOntoStreetName) {
  auto result = gurka::route(map, "A", "D", "auto");
  gurka::assert::raw::expect_maneuvers(result, {DirectionsLeg_Maneuver_Type_kStart,
                                                DirectionsLeg_Maneuver_Type_kRoundaboutEnter,
                                                DirectionsLeg_Maneuver_Type_kRoundaboutExit,
                                                DirectionsLeg_Maneuver_Type_kDestination});
  int maneuver_index = 1;

  // Verify the enter_roundabout instructions
  gurka::assert::raw::expect_instructions_at_maneuver_index(
      result, maneuver_index, "Enter Dupont Circle and take the 1st exit onto Homestead Lane.",
      "Enter Dupont Circle and take the 1st exit onto Homestead Lane.",
      "Enter Dupont Circle and take the 1st exit onto Homestead Lane.", "");

  // Verify the exit_roundabout instructions
  gurka::assert::raw::expect_instructions_at_maneuver_index(
      result, ++maneuver_index, "Exit the roundabout onto Homestead Lane.", "",
      "Exit the roundabout onto Homestead Lane. Then You will arrive at your destination.",
      "Continue for 200 meters.");
}

// enter_roundabout_verbal
// "11": Enter <STREET_NAMES> and take the <ORDINAL_VALUE> exit onto
// <ROUNDABOUT_EXIT_BEGIN_STREET_NAMES>."
TEST_F(InstructionsNamedRoundabout, RoundaboutOntoBeginStreetName) {
  auto result = gurka::route(map, "A", "G", "auto");
  gurka::assert::raw::expect_maneuvers(result, {DirectionsLeg_Maneuver_Type_kStart,
                                                DirectionsLeg_Maneuver_Type_kRoundaboutEnter,
                                                DirectionsLeg_Maneuver_Type_kRoundaboutExit,
                                                DirectionsLeg_Maneuver_Type_kDestination});
  int maneuver_index = 1;

  // Verify the enter_roundabout instructions
  gurka::assert::raw::expect_instructions_at_maneuver_index(
      result, maneuver_index,
      "Enter Dupont Circle and take the 2nd exit onto East Governor Road/A 1 East.",
      "Enter Dupont Circle and take the 2nd exit onto East Governor Road.",
      "Enter Dupont Circle and take the 2nd exit onto East Governor Road, A 1 East.", "");

  // Verify the exit_roundabout instructions
  gurka::assert::raw::expect_instructions_at_maneuver_index(
      result, ++maneuver_index,
      "Exit the roundabout onto East Governor Road/A 1 East. Continue on A 1 East.", "",
      "Exit the roundabout onto East Governor Road, A 1 East.",
      "Continue on A 1 East for 300 meters.");
}

// enter_roundabout_verbal
// "12": "Enter <STREET_NAMES> and take the <ORDINAL_VALUE> exit toward <TOWARD_SIGN>."
TEST_F(InstructionsNamedRoundabout, RoundaboutToward) {
  auto result = gurka::route(map, "I", "A", "auto");
  gurka::assert::raw::expect_maneuvers(result, {DirectionsLeg_Maneuver_Type_kStart,
                                                DirectionsLeg_Maneuver_Type_kRoundaboutEnter,
                                                DirectionsLeg_Maneuver_Type_kRoundaboutExit,
                                                DirectionsLeg_Maneuver_Type_kDestination});
  int maneuver_index = 1;

  // Verify the enter_roundabout instructions
  gurka::assert::raw::expect_instructions_at_maneuver_index(
      result, maneuver_index,
      "Enter Dupont Circle and take the 1st exit toward A 95/B 2/München/Kürten.",
      "Enter Dupont Circle and take the 1st exit toward A 95.",
      "Enter Dupont Circle and take the 1st exit toward A 95, München.", "");

  // Verify the exit_roundabout instructions
  gurka::assert::raw::expect_instructions_at_maneuver_index(
      result, ++maneuver_index, "Exit the roundabout toward A 95/B 2/München/Kürten.", "",
      "Exit the roundabout toward A 95, München. Then You will arrive at your destination.",
      "Continue for 200 meters.");
}