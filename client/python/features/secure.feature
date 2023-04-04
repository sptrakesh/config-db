Feature: Secure
  Simple test suite connecting to the TCP service over SSL

  Scenario: Auto closing SSL client
    Given ConfigDb service running on localhost on port 2020
    Then Client can be used to perform CRUD operations