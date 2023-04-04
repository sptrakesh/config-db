Feature: Resource
  Test illustrating use of the client as a auto-closing resource.

  Scenario: Auto closing resource
    Given ConfigDb service running on localhost on port 2022
    Then Client can be used as an auto-closing resource