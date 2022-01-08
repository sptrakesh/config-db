#!/bin/sh

url='https://localhost:6006/key/test/key'
if [ "$1" = "docker" ]
then
  url='https://localhost:6000/key/test/key'
fi

lurl='https://localhost:6006/list/test'
if [ "$1" = "docker" ]
then
  lurl='https://localhost:6000/list/test'
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

Put && Get && List && Delete
