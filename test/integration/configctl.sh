#!/bin/ksh

Set()
{
  resp=`../../cmake-build-debug/src/cli/configctl -a set -k /test -v value`
  if [ "$resp" = "Set value for key /test" ]
  then
    echo "Set value"
  else
    echo "Error setting value"
  fi
}

Get()
{
  resp=`../../cmake-build-debug/src/cli/configctl -a get -k /test`
  if [ "$resp" = "value" ]
  then
    echo "Got value"
  else
    echo "Error retrieving value"
  fi
}

List()
{
  resp=`../../cmake-build-debug/src/cli/configctl -a list -k /`
  if [ "$resp" = "$1" ]
  then
    echo "List returned expected value"
  else
    echo "Error listing root"
  fi
}

Move()
{
  resp=`../../cmake-build-debug/src/cli/configctl -a move -k /test -v /test1`
  if [ "$resp" = "Moved key /test to /test1" ]
  then
    echo "Moved key"
  else
    echo "Error moving key"
  fi
}

Delete()
{
  resp=`../../cmake-build-debug/src/cli/configctl -a delete -k /test1`
  if [ "$resp" = "Removed key /test1" ]
  then
    echo "Removed key"
  else
    echo "Error removing key"
  fi
}

cd `dirname $0`
Set && Get && List "test" && Move && List "test1" && Delete