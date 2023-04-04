Feature: CRUD
  CRUD test suite for the config-db client.

  Scenario: API CRUD test
    Given A client instance
    Then Setting a key-value pair
    And Reading the key
    And Updating the value
    And Reading the key again
    And Update rejected due to if not exists
    And Removing the key
    And Reading the key after remove
    And Removing the key again