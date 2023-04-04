Feature: Batch
  Batch operations test suite.

  Scenario: API Batch test
    Given A client instance
    Then Creating keys
    And Retrieving the keys
    And Updating the keys
    And Retrieving the modified keys
    And Listing multiple paths
    And Removing the keys
    And Retrieving the deleted keys
    And Listing deleted multiple paths
