# RV-321 Codec Transport Channel Guard Review

## Scope

- Review ID: `RV-321`
- Scope: `codec invalid transport channel reject`
- Date: 2026-05-06

## Findings

1. `TC-CODEC-023` verifies that decode rejects an otherwise well-formed frame with `INVALID_CHANNEL` when `eChannelId` is `RSRX_TRANSPORT_CHANNEL_INVALID`.
2. The production guard admits only configured transport channel ids from `PRIMARY` through `REDUNDANT`.
3. This prevents an invalid transport origin from being converted into a typed decoded protocol message.

## Disposition

- Pass.
- Codec decode now has explicit transport channel admission evidence.
