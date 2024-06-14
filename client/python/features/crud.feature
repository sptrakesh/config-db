Feature: CRUD
  CRUD test suite for the config-db client.

  Scenario: API CRUD test
    Given A client instance
    Then Setting a key-value pair
    And Reading the key
    And Listing the tree at "/" has "1" children
    And Updating the value
    And Reading the key again
    And Update rejected due to if not exists
    And Removing the key
    And Reading the key after remove
    And Removing the key again
    And Setting a cache key-value pair
    And Listing the tree at "/" has "0" children
    And Reading the key
    And Removing the key
