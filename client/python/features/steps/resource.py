from behave import given, then
from behave.api.async_step import async_run_until_complete
from behave.runner import Context
from hamcrest import assert_that, equal_to, empty

from client import Client as _Client


@given("ConfigDb service running on {host} on port {port:d}")
def step_impl(context: Context, host: str, port: int):
    context.host = host
    context.port = port


@then("Client can be used as an auto-closing resource")
@async_run_until_complete
async def step_impl(context: Context):
    assert_that(hasattr(context, "host"), "Host not set")
    assert_that(hasattr(context, "port"), "Port not set")

    async with _Client(host=context.host, port=context.port) as client:
        _key = "key"
        res = await client.set(_key, "value")
        assert_that(res, equal_to(True), "Setting key did not return true")

        res = await client.get(_key)
        assert_that(res, equal_to("value"), "Getting key did not return value")

        res = await client.set(_key, "value modified")
        assert_that(res, equal_to(True), "Updating key did not return true")

        res = await client.set(_key, "value", if_not_exists=True)
        assert_that(res, equal_to(False), "Update with existing key did not fail")

        res = await client.remove(_key)
        assert_that(res, equal_to(True), "Removing key did not return true")

        res = await client.get(_key)
        assert_that(res, equal_to(None), "Getting deleted key did not return None")

        res = await client.remove(_key)
        assert_that(res, equal_to(True), "Removing key did not return true")
