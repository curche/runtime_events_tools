#define CAML_NAME_SPACE

#include <caml/alloc.h>
#include <caml/runtime_events.h>
#include <caml/runtime_events_consumer.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>

#include <assert.h>

struct counters {
    int runtime_events_trigger_counters[52];
};

struct caml_runtime_events_cursor* olly_start_runtime_events(void) {
    CAMLparam0();
    caml_runtime_events_start();
    // CAMLlocal1(cursor);

    struct caml_runtime_events_cursor* cursor;

    runtime_events_error res = caml_runtime_events_create_cursor(NULL, -1, &cursor);

    if( res != E_SUCCESS ) {
        caml_failwith("Runtime_events.start_runtime_events: invalid or non-existent cursor");
    }

    return cursor;
}

int ev_begin(int domain_id, void* callback_data,
                uint64_t timestamp, ev_runtime_phase phase) {
    struct counters* tmp_counters = (struct counters*)callback_data;
    tmp_counters->runtime_events_trigger_counters[(int)phase]++;

    return 1;
}

value olly_get_event_counts(struct caml_runtime_events_cursor* cursor) {
    CAMLparam1(cursor); 
    CAMLlocal3(res_list, res_pair, res_phase);
    runtime_events_error res;
    uintnat events_consumed;

    struct counters tmp_counters = { 0 };

    res_list = Val_emptylist;

    // struct caml_runtime_events_cursor* cursor;

    // res = caml_runtime_events_create_cursor(NULL, -1, &cursor);

    // if( res != E_SUCCESS ) {
    //     caml_failwith("Runtime_events.get_event_counts: invalid or non-existent cursor");
    // }

    caml_runtime_events_set_runtime_begin(cursor, &ev_begin);
    // caml_runtime_events_set_runtime_end(cursor, &ev_end);

    res = caml_runtime_events_read_poll(cursor, &tmp_counters, 0,
                                   &events_consumed);

    if( res != E_SUCCESS ) {
        caml_failwith("Runtime_events.get_event_counts: error reading from rings");
    }

    printf("shared frequency list once\n"); // NOTE: this only prints once!
    for(int i = 0; i < 50; i++) {
        // create a (runtime_phase, int) pair and add it to the list
        // call Runtime_events.runtime_phase_name to get the string name in OCaml

        res_phase = Val_int(i); // create a runtime_phase variant value from integer
        res_pair = caml_alloc_2(0, res_phase, Val_int(tmp_counters.runtime_events_trigger_counters[i]));
        res_list = caml_alloc_2(Tag_cons, res_pair, res_list);
    }

    caml_runtime_events_free_cursor(cursor);

    CAMLreturn(res_list);
}
