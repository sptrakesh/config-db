#!/bin/sh

url='http://localhost:6026/key/test/key'
if [ "$1" = "docker" ]
then
  url='http://localhost:6020/key/test/key'
fi

lurl='http://localhost:6026/list/test'
if [ "$1" = "docker" ]
then
  lurl='http://localhost:6020/list/test'
fi

Put()
{
  echo "PUT request for key test"
  value=`curl -s -k --http2-prior-knowledge -XPUT -H "content-type: text/plain" -d "value" $url | jq -r .code`
  if [ "$value" = "200" ]
  then
    echo "Saved key-value pair"
  else
    echo "Error saving key-value pair"
  fi
}

PutNotExists()
{
  echo "PUT request for key test with if_not_exists"
  value=`curl -s -k --http2-prior-knowledge -XPUT -H "content-type: text/plain" -H "x-config-db-if-not-exists: true" -d "value" $url | jq -r .code`
  if [ "$value" = "412" ]
  then
    echo "Updating existing key-value pair rejected due to header"
  else
    echo "Error.  Updated key-value pair"
  fi
}

Get()
{
  echo "GET request for key test"
  value=`curl -s -k --http2-prior-knowledge $url | jq -r .value`
  if [ "$value" = "value" ]
  then
    echo "Expected value retrieved"
  else
    echo "Unexpected value [$value] received"
  fi
}

List()
{
  echo "GET request for listing child nodes"
  value=`curl -s -k --http2-prior-knowledge $lurl | jq -r .children[0]`
  if [ "$value" = "key" ]
  then
    echo "Expected value retrieved"
  else
    echo "Unexpected value ($value) received"
  fi
}

Delete()
{
  echo "DELETE request for key test"
  value=`curl -s -k --http2-prior-knowledge -XDELETE $url | jq -r .code`
  if [ "$value" = "200" ]
  then
    echo "Deleted key-value pair"
  else
    echo "Error deleting key-value pair"
  fi
}

Put && Get && List && PutNotExists && Delete
