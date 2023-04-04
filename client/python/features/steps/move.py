from behave import then, step
from behave.api.async_step import async_run_until_complete
from behave.runner import Context
from hamcrest import assert_that, equal_to, empty

_key1 = "/key1/key2/key3"
_key2 = "/key1/key2/key4"
_key3 = "/key1/key21/key3"
_key4 = "/key1/key21/key4"

_dest1 = "/key/key2/key3"
_dest2 = "/key/key2/key4"
_dest3 = "/key/key21/key3"
_dest4 = "/key/key21/key4"


@then("Creating the original keys")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.set_key_values([(_key1, "value"), (_key2, "value"), (_key3, "value"), (_key4, "value")])
    assert_that(res, equal_to(True), "Setting batch of key-values failed")


@step("Moving the keys")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.move_keys([
        (_key1, _dest1),
        (_key2, _dest2),
        (_key3, _dest3),
        (_key4, _dest4)
    ])
    assert_that(res, equal_to(True), "Moving batch of key-values failed")


@step("Retrieving the moved keys")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.get_keys([_key1, _key2, _key3, _key4])
    assert_that(res, "Getting keys did not return results")
    for kv in res:
        assert_that(kv[1], equal_to(None), f"Deleted key {kv[0]} returned value")


@step("Retrieving the destination keys")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.get_keys([_dest1, _dest2, _dest3, _dest4])
    assert_that(res, "Getting keys did not return results")
    assert_that(len(res), equal_to(4), "Getting keys did not return correct number of values")
    for kv in res:
        assert_that(kv[1], equal_to("value"), f"Value for key {kv[0]} not as set")


@step("Listing destination paths")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.list_paths(["/", "/key", "/key/key2", "/key/key21"])
    assert_that(res, "Listing keys did not return results")
    assert_that(len(res), equal_to(4), "Listing paths did not return correct number of values")

    assert_that(res[0][0], equal_to("/"), "First path not same as specified")
    assert_that(res[0][1], equal_to(["key"]), "First path children not as specified")

    assert_that(res[1][0], equal_to("/key"), "Second path not same as specified")
    assert_that(res[1][1], equal_to(["key2", "key21"]), "Second path children not as specified")

    assert_that(res[2][0], equal_to("/key/key2"), "Third path not same as specified")
    assert_that(res[2][1], equal_to(["key3", "key4"]), "Third path children not as specified")

    assert_that(res[3][0], equal_to("/key/key21"), "Fourth path not same as specified")
    assert_that(res[3][1], equal_to(["key3", "key4"]), "Fourth path children not as specified")


@step("Removing the destination keys")
@async_run_until_complete
async def step_impl(context: Context):
    res = await context.client.remove_keys([_dest1, _dest2, _dest3, _dest4])
    assert_that(res, equal_to(True), "Deleting batch of key-values failed")
