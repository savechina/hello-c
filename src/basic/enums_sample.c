#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "basic/enums_sample.h"

/* ---- Section 1: Basic enum definition ---- */

enum Weekday { MON, TUE, WED, THU, FRI, SAT, SUN };

static void enum_basic(void) {
    printf("==== [sample] Basic Enum Definition ====\n");

    enum Weekday today = MON;
    printf("  MON = %d, TUE = %d, SUN = %d\n", MON, TUE, SUN);
    printf("  today = %d (%s)\n", today, today == MON ? "Monday" : "other");

    printf("  sizeof(enum Weekday) = %zu\n", sizeof(enum Weekday));
    printf("  sizeof(int)          = %zu\n", sizeof(int));
}

/* ---- Section 2: Enum with explicit values ---- */

enum ErrorCode {
    ERR_OK = 0,
    ERR_INVALID_ARG,
    ERR_NULL_PTR,
    ERR_TIMEOUT = 100,
    ERR_IO
};

static void enum_explicit_values(void) {
    printf("\n==== [sample] Enum Explicit Values ====\n");

    printf("  ERR_OK           = %d\n", ERR_OK);
    printf("  ERR_INVALID_ARG  = %d (auto-incremented from 0)\n", ERR_INVALID_ARG);
    printf("  ERR_NULL_PTR     = %d (auto-incremented)\n", ERR_NULL_PTR);
    printf("  ERR_TIMEOUT      = %d (explicitly set)\n", ERR_TIMEOUT);
    printf("  ERR_IO           = %d (auto-incremented from 100)\n", ERR_IO);
}

/* ---- Section 3: Enum as function parameter ---- */

typedef enum LogLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

static void log_message(LogLevel level, const char *msg) {
    const char *prefix = "";
    switch (level) {
        case LOG_DEBUG: prefix = "DEBUG"; break;
        case LOG_INFO:  prefix = "INFO";  break;
        case LOG_WARN:  prefix = "WARN";  break;
        case LOG_ERROR: prefix = "ERROR"; break;
        default:        prefix = "????";  break;
    }
    printf("  [%s] %s\n", prefix, msg);
}

static void enum_as_parameter(void) {
    printf("\n==== [sample] Enum as Function Parameter ====\n");

    log_message(LOG_INFO, "Program starting");
    log_message(LOG_DEBUG, "Debug info (visible only if level permits)");
    log_message(LOG_WARN, "Disk space is low");
    log_message(LOG_ERROR, "Failed to open file");
}

/* ---- Section 4: Enum validation (error-first) ---- */

static bool is_valid_log_level(LogLevel level) {
    return level >= LOG_DEBUG && level <= LOG_ERROR;
}

static void enum_validation(void) {
    printf("\n==== [sample] Enum Validation ====\n");

    LogLevel safe_level = LOG_WARN;
    printf("  Valid level %d: %s\n", safe_level,
           is_valid_log_level(safe_level) ? "OK" : "INVALID");

    /* Demonstrate what happens with an out-of-range value */
    LogLevel bad_level = (LogLevel)99;
    printf("  Out-of-range level %d: %s\n", bad_level,
           is_valid_log_level(bad_level) ? "OK" : "INVALID");
    printf("  <-- C allows casting any int to enum; validation catches it!\n");
}

/* ---- Section 5: Tagged Union (enum + union) ---- */

typedef union VariantData {
    int32_t i;
    double  d;
    const char *s;
} VariantData;

enum ValueKind { VK_INT, VK_DOUBLE, VK_STRING };

struct Variant {
    enum ValueKind tag;
    union VariantData data;
};

static struct Variant variant_int(int32_t val) {
    struct Variant v;
    v.tag = VK_INT;
    v.data.i = val;
    return v;
}

static struct Variant variant_double(double val) {
    struct Variant v;
    v.tag = VK_DOUBLE;
    v.data.d = val;
    return v;
}

static struct Variant variant_string(const char *val) {
    struct Variant v;
    v.tag = VK_STRING;
    v.data.s = val;
    return v;
}

static void print_variant(const struct Variant *v) {
    switch (v->tag) {
        case VK_INT:
            printf("    int:    %d\n", v->data.i);
            break;
        case VK_DOUBLE:
            printf("    double: %.2f\n", v->data.d);
            break;
        case VK_STRING:
            printf("    string: \"%s\"\n", v->data.s);
            break;
        default:
            printf("    unknown type (%d)\n", v->tag);
            break;
    }
}

static void tagged_union_demo(void) {
    printf("\n==== [sample] Tagged Union (enum + union) ====\n");

    struct Variant a = variant_int(42);
    struct Variant b = variant_double(3.14);
    struct Variant c = variant_string("hello");

    printf("  a: "); print_variant(&a);
    printf("  b: "); print_variant(&b);
    printf("  c: "); print_variant(&c);

    printf("\n  sizeof(VariantData)      = %zu\n", sizeof(VariantData));
    printf("  sizeof(struct Variant)    = %zu (tag + union, with padding)\n", sizeof(struct Variant));
}

/* ---- Section 6: Practical - state machine ---- */

typedef enum LightState {
    STATE_OFF,
    STATE_DIM,
    STATE_BRIGHT
} LightState;

static LightState next_light_state(LightState current) {
    switch (current) {
        case STATE_OFF:    return STATE_DIM;
        case STATE_DIM:    return STATE_BRIGHT;
        case STATE_BRIGHT: return STATE_OFF;
        default:           return STATE_OFF;
    }
}

static const char *light_name(LightState s) {
    switch (s) {
        case STATE_OFF:    return "OFF";
        case STATE_DIM:    return "DIM";
        case STATE_BRIGHT: return "BRIGHT";
        default:           return "UNKNOWN";
    }
}

static void state_machine_demo(void) {
    printf("\n==== [sample] State Machine (Light Switch) ====\n");

    LightState state = STATE_OFF;
    for (int i = 0; i < 5; i++) {
        printf("  current: %s\n", light_name(state));
        state = next_light_state(state);
    }
    printf("  final: %s\n", light_name(state));
}

/* ---- Section 7: Bit flags using enum ---- */

typedef enum FileAccess {
    ACCESS_READ    = 1 << 0,
    ACCESS_WRITE   = 1 << 1,
    ACCESS_EXECUTE = 1 << 2
} FileAccess;

static void bit_flags_demo(void) {
    printf("\n==== [sample] Bit Flags (enum with powers of 2) ====\n");

    FileAccess permissions = ACCESS_READ | ACCESS_WRITE;

    printf("  permissions = %d\n", permissions);
    printf("  has READ?  %s\n", (permissions & ACCESS_READ)    ? "yes" : "no");
    printf("  has WRITE?  %s\n", (permissions & ACCESS_WRITE)   ? "yes" : "no");
    printf("  has EXEC?   %s\n", (permissions & ACCESS_EXECUTE) ? "yes" : "no");

    permissions |= ACCESS_EXECUTE;
    printf("  after adding EXECUTE: %d\n", permissions);
    printf("  has EXEC now? %s\n", (permissions & ACCESS_EXECUTE) ? "yes" : "no");
}

/* ---- Public entry ---- */

int main_enums_sample(void) {
    enum_basic();
    enum_explicit_values();
    enum_as_parameter();
    enum_validation();
    tagged_union_demo();
    state_machine_demo();
    bit_flags_demo();

    printf("\nenums sample done.\n");
    return 0;
}
