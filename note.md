GEP write-back pattern for mutating nested growable fields (arrays/strings) inside structs.

The rule: any op that can grow or reallocate a value (push_back, extend, del, pop, etc) returns a new header (ptr, length, capacity). That new header only exists in your local SSA value until you explicitly write it back to wherever it's stored. If you skip the write-back, the parent struct still points at the old, possibly stale, backing memory.

Pattern for something like val.field.field[idx].push_back(x):

1. Walk down to the target field's own address using a single GEP call with the full index chain: GEP(val, field_idx, field_idx2, idx). This gives you the address of the growable field slot itself, not the address of an element inside it. This address is stable even though the array's internal data pointer can change on realloc.

2. Load the current header from that address.

3. Call push_back (or extend/del/pop) on the loaded header. This may reallocate and returns a new header.

4. Store the new header back to the same GEP address from step 1.

You only need one GEP call for the whole chain, computed once, reused for both the load and the store. No need to recompute it or GEP again after the mutation, since the field slot's address never moves, only what is stored there changes.

Do not reuse a GEP address, or any value derived from one, after a call that might reallocate something upstream of it in the same chain. If you have two separate accesses sharing a prefix (eg reading val.field[idx] and separately calling val.field.push_back elsewhere in the same statement), and the push_back can reallocate field, any earlier GEP into field[idx] is now stale. Recompute it fresh after the mutation if you need it again.

For a plain scalar terminal write with no growth (eg val.field[idx].field2 = 5), skip the load and header dance entirely. Just GEP straight to the scalar slot and store the value directly. No write-back needed since you are mutating shared backing memory in place, not replacing a header.