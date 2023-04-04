from behave import given, then, step
from behave.api.async_step import async_run_until_complete
from behave.runner import Context
from hamcrest import assert_that, equal_to

_key = "key"


@given("A client instance")
def step_impl(context: Context):
    assert_that(hasattr(context, "client"), "Client not set")


@then("Setting a key-value pair")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.set(_key, "value")
    assert_that(res, equal_to(True), "Setting key did not return true")


@step("Reading the key")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.get(_key)
    assert_that(res, equal_to("value"), "Getting key did not return value")


@step("Updating the value")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.set(_key, "value modified")
    assert_that(res, equal_to(True), "Updating key did not return true")


@step("Reading the key again")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.get(_key)
    assert_that(res, equal_to("value modified"), "Getting key did not return modified value")


@step("Update rejected due to if not exists")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.set(_key, "value", if_not_exists=True)
    assert_that(res, equal_to(False), "Update with existing key did not fail")


@step("Removing the key")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.remove(_key)
    assert_that(res, equal_to(True), "Removing key did not return true")


@step("Reading the key after remove")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.get(_key)
    assert_that(res, equal_to(None), "Getting deleted key did not return None")


@step("Removing the key again")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.remove(_key)
    assert_that(res, equal_to(True), "Removing key did not return true")
