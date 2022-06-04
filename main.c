#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <wchar.h>

struct wh_object_t {
    uint_fast8_t type;
    uintptr_t data;
};

struct wh_pair_t {
    struct wh_object_t * first;
    struct wh_object_t * second;
};

#define wh_integer 0
#define wh_number 1
#define wh_string 2
#define wh_symbol 3
#define wh_pair 4
#define wh_boolean 5
#define wh_nothing 6

#define object struct wh_object_t *
#define as_object(o) ((object) (o))
#define type(o) as_object(o)->type
#define data(o) as_object(o)->data
#define as_data(x) ((uintptr_t) (x))

#define integer_type uintptr_t
#define number_type double *
#define string_type wchar_t *
#define symbol_type wchar_t *

#define object_allocation malloc(sizeof(struct wh_object_t))
#define pair_allocation malloc(sizeof(struct wh_pair_t))

#define pair struct wh_pair_t *
#define as_pair(o) ((struct wh_pair_t *) (o))
#define first(o) as_pair(data(o))->first
#define second(o) as_pair(data(o))->second

object integer(integer_type x) {
    object o = object_allocation;
    type(o) = wh_integer;
    data(o) = as_data(x);
    return o;
}

object symbol(const wchar_t * name) {
    object o = object_allocation;
    type(o) = wh_symbol;
    data(o) = as_data(name);
    return o;
}

void display_object(object obj) {
    if (obj == NULL) {
        printf("nil");
        return;
    }
    object cur;
    switch (type(obj)) {
        case wh_integer:
            printf("%ld", (long) data(obj));
            break;
        case wh_number:
            printf("%f", *((double *) data(obj)));
            break;
        case wh_string:
            printf("%ls", (wchar_t *) data(obj));
            break;
        case wh_symbol:
            printf("%ls", (wchar_t *) data(obj));
            break;
        case wh_pair:
            printf("(");
            cur = obj;
            while (type(cur) != wh_nothing) {
                display_object(first(cur));
                cur = second(cur);
                if (type(cur) == wh_pair) {
                    printf(" ");
                }
            }
            if (type(cur) != wh_nothing) display_object(cur);
            printf(")");
            break;
        case wh_boolean:
            printf("%s", data(obj) ? "true" : "false");
            break;
        case wh_nothing:
            printf("nil");
            break;
    }
}

const wchar_t * quote_symbol = L"quote";

object parse_symbol(wchar_t * str, size_t * pos) {
    size_t i = *pos;
    while ( str[i] != L'\0' &&
            str[i] != L' '  &&
            str[i] != L'\n' &&
            str[i] != L'\t' &&
            str[i] != L'('  &&
            str[i] != L')'  &&
            str[i] != L'\'' &&
            str[i] != L'\r'    ) {
        i++;
    }
    size_t len = i - *pos;
    symbol_type sym = malloc(sizeof(wchar_t) * (len + 1));
    memcpy(sym, str + *pos, sizeof(wchar_t) * len);
    sym[len] = L'\0';
    *pos = i;
    return symbol(sym);
}

object list_finish(object p_head, object cur) {
    object ret = second(p_head);
    free(as_pair(data(p_head)));
    free(p_head);
    second(cur) = object_allocation;
    type(second(cur)) = wh_nothing;
    data(second(cur)) = as_data(NULL);
    return ret;
}

object list_create() {
    object list = object_allocation;
    type(list) = wh_pair;
    data(list) = as_data(pair_allocation);
    return list;
}

object list_append(object cur, object obj) {
    object next = object_allocation;
    type(next) = wh_pair;
    data(next) = as_data(pair_allocation);
    first(next) = obj;
    second(next) = NULL;
    second(cur) = next;
    return next;
}

object parse(wchar_t * str, size_t * pos) {
    object cur = list_create();
    object head = cur;
    size_t len = wcslen(str);
    while (*pos < len) {
        switch (str[*pos]) {
            case L'(':
                *pos += 1;
                cur = list_append(cur, parse(str, pos));
                break;
            case L')':
                *pos += 1;
                return list_finish(head, cur);
            case L' ':
            case L'\n':
            case L'\t':
            case L'\r':
                *pos += 1;
                break;
            case L'\'':
                *pos += 1;
                object quoted_cur = list_create();
                object quoted_head = quoted_cur;
                quoted_cur = list_append(quoted_cur, symbol(quote_symbol));
                if (str[*pos] == L'(') {
                    *pos += 1;
                    quoted_cur = list_append(quoted_cur, parse(str, pos));
                } else {
                    quoted_cur = list_append(quoted_cur, parse_symbol(str, pos));
                }
                cur = list_append(cur, list_finish(quoted_head, quoted_cur));
                break;
            default:
                cur = list_append(cur, parse_symbol(str, pos));
                break;
        }
    }
    return list_finish(head, cur);
}

int main() {
    wchar_t * str = L"(+ 1 2)";
    size_t pos = 0;
    object obj = parse(str, &pos);
    display_object(obj);
    return 0;
}
