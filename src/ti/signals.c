/*
 * signals.c
 */
#include <uv.h>
#include <signal.h>
#include <stdlib.h>
#include <ti/connect.h>
#include <ti/events.h>
#include <ti/signals.h>
#include <ti.h>
#include <util/logger.h>

static void signals__handler(uv_signal_t * sig, int signum);

#define signals__nsigs 6
static const int signals__signms[signals__nsigs] = {
        SIGHUP,
        SIGINT,
        SIGTERM,
        SIGSEGV,
        SIGABRT,
        SIGPIPE
};

static uv_signal_t signals[signals__nsigs] = {0};

int ti_signals_init(void)
{
    /* bind signals to the event loop */
    for (int i = 0; i < signals__nsigs; i++)
    {
        if (uv_signal_init(ti()->loop, &signals[i]) ||
            uv_signal_start(&signals[i], signals__handler, signals__signms[i]))
        {
            return -1;
        }
    }
    return 0;
}

static void signals__handler(uv_signal_t * UNUSED(sig), int signum)
{
    if (signum == SIGPIPE)
    {
        log_warning("signal (%d) received, probably a connection was lost");
        return;
    }

    if (ti()->flags & TI_FLAG_SIGNAL)
    {
        log_error("received second signal (%s), abort", strsignal(signum));
        abort();
    }

    ti()->flags |= TI_FLAG_SIGNAL;

    if (signum == SIGINT || signum == SIGTERM || signum == SIGHUP)
        log_warning("received stop signal (%s)", strsignal(signum));
    else
        log_critical("received stop signal (%s)", strsignal(signum));

//    ti_maint_stop(ti()->maint);
    ti_connect_stop();
    ti_events_stop();

    uv_stop(ti()->loop);
}