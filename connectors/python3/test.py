import time
import asyncio
import logging
import pprint
import signal
from thingsdb.client import Client
from thingsdb.exceptions import ThingsDBError


interrupted = False


async def test():
    client = Client()
    await client.connect('localhost')

    # Authenticte
    try:
        await client.authenticate('iris', 'siri')
    except ThingsDBError as e:
        print(e)

    client.use('test')

    start = time.time()
    try:
        res = await client.watch([3])
    except ThingsDBError as e:
        print(f"{e.__class__.__name__}: {e}")
    else:
        print('Time: {}'.format(time.time() - start))
        pprint.pprint(res)

    await asyncio.sleep(0.5)
    print('-----------------------------------------------------------------')

    start = time.time()
    try:
        res = await client.query(r'''
        /*
         * test query
         */
        /* del_collection('test2'); */
        new_node('secret', '0.0.0.0', 9001);
        new_collection('test');

        ''', timeout=2, target=0)
    except ThingsDBError as e:
        print(f"{e.__class__.__name__}: {e}")
    else:
        print('Time: {}'.format(time.time() - start))
        pprint.pprint(res)

    await asyncio.sleep(0.5)
    print('-----------------------------------------------------------------')

    start = time.time()
    try:
        res = await client.query(r'''



        ''', blobs=["bla"], timeout=2)
    except ThingsDBError as e:
        print(f"{e.__class__.__name__}: {e}")
    else:
        print('Time: {}'.format(time.time() - start))
        pprint.pprint(res)

    while not interrupted:
        await asyncio.sleep(0.1)

    print('-----------------------------------------------------------------')


def signal_handler(signal, frame):
    print('Quit...')
    global interrupted
    interrupted = True


if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal_handler)

    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG)
    loop = asyncio.get_event_loop()
    loop.run_until_complete(test())
