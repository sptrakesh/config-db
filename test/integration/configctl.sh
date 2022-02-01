#!/bin/ksh

Set()
{
  resp=`../../cmake-build-debug/src/cli/configctl -a set -k /test -v value`
  if [ $? -ne 0 ]
  then
    echo "Error setting value"
    return
  fi

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
  if [ $? -ne 0 ]
  then
    echo "Error retrieving value"
    return
  fi

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
  if [ $? -ne 0 ]
  then
    echo "Error moving key"
    return
  fi

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
  if [ $? -ne 0 ]
  then
    echo "Error removing key"
    return
  fi

  if [ "$resp" = "Removed key /test1" ]
  then
    echo "Removed key"
  else
    echo "Error removing key"
  fi
}

Invalid()
{
  resp=`../../cmake-build-debug/src/cli/configctl -a get -k /test`
  #if [ $? -ne 4 ]
  if [ $? -eq 0 ]
  then
    echo "Invalid response code when retrieving deleted key"
    return
  fi
}

cd `dirname $0`
Set && Get && List "test" && Move && List "test1" && Delete && Invalid