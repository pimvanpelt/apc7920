#include "rpc.h"
#include "channel.h"

static void rpc_log(struct mg_rpc_request_info *ri, struct mg_str args) {
  LOG(LL_INFO, ("tag=%.*s src=%.*s method=%.*s args='%.*s'",
                ri->tag.len, ri->tag.p, ri->src.len, ri->src.p,
                ri->method.len, ri->method.p, args.len, args.p));
}

static void rpc_channel_toggle_handler(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args) {
  int     idx;

  rpc_log(ri, args);

  if (json_scanf(args.p, args.len, ri->args_fmt, &idx) != 1) {
    mg_rpc_send_errorf(ri, 400, "idx is required");
    ri = NULL;
    return;
  }

  if (idx<0 || idx>7) {
    mg_rpc_send_errorf(ri, 400, "idx must be [0,8>");
    ri = NULL;
    return;
  }

  channel_toggle(idx);
  mg_rpc_send_responsef(ri, "{idx: %d, value: %d}", idx, channel_get(idx));
  ri = NULL;

  (void)cb_arg;
  (void)fi;
}

static void rpc_channel_get_handler(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args) {
  int     idx;

  rpc_log(ri, args);

  if (json_scanf(args.p, args.len, ri->args_fmt, &idx) != 1) {
    mg_rpc_send_errorf(ri, 400, "idx is required");
    ri = NULL;
    return;
  }

  if (idx<0 || idx>7) {
    mg_rpc_send_errorf(ri, 400, "idx must be [0,8>");
    ri = NULL;
    return;
  }

  mg_rpc_send_responsef(ri, "{idx: %d, value: %d}", idx, channel_get(idx));
  ri = NULL;

  (void)cb_arg;
  (void)fi;
}

static void rpc_channel_set_handler(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args) {
  int     idx;
  int     value;

  rpc_log(ri, args);

  if (json_scanf(args.p, args.len, ri->args_fmt, &idx, &value) != 2) {
    mg_rpc_send_errorf(ri, 400, "idx and value are required");
    ri = NULL;
    return;
  }

  if (idx<0 || idx>7) {
    mg_rpc_send_errorf(ri, 400, "idx must be [0,8>");
    ri = NULL;
    return;
  }

  channel_set(idx, (bool)value);
  mg_rpc_send_responsef(ri, "{idx: %d, value: %d}", idx, channel_get(idx));
  ri = NULL;

  (void)cb_arg;
  (void)fi;
}

void rpc_init() {
  struct mg_rpc *c = mgos_rpc_get_global();

  mg_rpc_add_handler(c, "Channel.Toggle", "{idx: %d}", rpc_channel_toggle_handler, NULL);
  mg_rpc_add_handler(c, "Channel.Get", "{idx: %d}", rpc_channel_get_handler, NULL);
  mg_rpc_add_handler(c, "Channel.Set", "{idx: %d, value: %d}", rpc_channel_set_handler, NULL);
}
