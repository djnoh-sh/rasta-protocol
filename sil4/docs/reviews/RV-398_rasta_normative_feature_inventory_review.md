# RV-398 RaSTA Normative Feature Inventory Review

## Scope

- Review item: `INV-RASTA-001`
- Risk focus: `R-006`
- Roadmap focus: separate mandatory RaSTA parity work from project-specific security extensions.

## Review Questions

1. Does the inventory distinguish RaSTA-defined PDU/checksum/timestamp work from optional/project-specific MAC security extension work?
2. Does the inventory avoid claiming full RaSTA PDU/checksum/timestamp parity for the current skeleton codec?
3. Does the roadmap now point to RaSTA parity definition before implementation growth?

## Findings

- `INV-RASTA-001` identifies SR PDU fields, message types, disconnect reasons, SR checksum/hash profiles, redundancy PDU/CRC profiles, timestamp validation, sequence/confirmation rules, and sender/receiver authenticity as RaSTA parity backlog.
- MAC/HMAC-style security is classified as a project extension unless a controlled RaSTA/customer requirement makes it mandatory.
- AM263Px hardware CRC/crypto acceleration is kept as target adapter/evidence work, not portable core logic.
- `roadmap_status.md` now separates `RaSTA PDU/checksum/timestamp parity` from `MAC/security extension`.

## Conclusion

Pass. The roadmap now has a cleaner basis for implementing RaSTA-defined protocol parity before optional security extensions.

## Residual Risk

- The current inventory is based on repository source/analysis documents. It must be updated with exact official RaSTA specification clause references when the controlled source document is added to the evidence set.
