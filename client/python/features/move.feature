Feature: Move
  Test suite around moving keys from one path to another.

  Scenario: API Move test
    Given A client instance
    Then Creating the original keys
    And Moving the keys
    And Retrieving the moved keys
    And Retrieving the destination keys
    And Listing destination paths
    And Removing the destination keys
