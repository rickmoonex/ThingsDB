# ThingsDB

## TODO list

- [x] Refactor
    - [x] database -> collection
    - [x] user_new etc -> new_user
    - [x] ti_res_t & ti_root_t  -> ti_query_t
- [ ] Language
    - [x] Primitives
        - [x] `false`
        - [x] `nil`
        - [x] `true`
        - [x] `float`
        - [x] `int`
        - [x] `string`
        - [x] `regex`
    - [x] Thing
    - [x] Array
    - [ ] Functions:
        - [ ] `blob`
            - [x] array implementation
            - [ ] map implementation
        - [x] `endswith`
        - [x] `filter`
        - [x] `get`
        - [x] `id`
        - [x] `lower`
        - [x] `map`
        - [x] `match`
        - [x] `now`
        - [x] `ret`
        - [x] `startswith`
        - [x] `thing`
        - [x] `upper`
        - [x] `del`
        - [x] `push`
        - [ ] `rename`
        - [x] `set`
        - [ ] `splice`
        - [x] `unset`
- [x] Redundancy as initial argument together with --init ?
- [ ] Task generating
    - [x] `assign`
    - [x] `del`
    - [x] `push`
    - [ ] `rename`
    - [x] `set`
    - [ ] `splice`
    - [x] `unset`
- [x] Events without tasks could be saved smaller
- [ ] Jobs processing from `EPKG`
    - [x] `assign`
    - [x] `del`
    - [ ] `push`
    - [ ] `rename`
    - [ ] `set`
    - [ ] `splice`
    - [ ] `unset`
- [x] Value implementation
    - [x] Bool
    - [x] Int
    - [x] Float
    - [x] Nil
    - [x] Thing
    - [x] Array
    - [x] Tuple
    - [x] Things
    - [x] Regex
    - [x] Arrow
    - [x] Raw
    - [x] Qp
- [x] Build first event on init
- [ ] Storing
    - [ ] Full storage on disk
        - [x] Status
        - [x] Nodes
        - [x] Databases
        - [x] Access
        - [ ] Things
            - [x] Skeleton
            - [ ] Data
            - [ ] Attributes
        - [x] Users
    - [x] Archive storing
        - [x] Store in away mode
        - [x] Load on startup (Required Jobs implementation for full coverage)
- [ ] Multi node
- [ ] Root
    - [ ] functions
        - [ ] `collection`
        - [ ] `collections`
        - [ ] `del_collection`
        - [ ] `del_node` ?? -> very hard
        - [x] `del_user`
        - [x] `grant`
        - [x] `new_collection`
        - [ ] `new_node`
        - [x] `new_user`
        - [ ] `node`
        - [ ] `nodes`
        - [ ] `reset_counters`
        - [x] `revoke`
        - [x] `user`
        - [ ] `users`
    - [ ] jobs
        - [ ] `del_collection`
        - [ ] `del_node` ?? -> very hard
        - [x] `del_user`
        - [x] `grant`
        - [x] `new_collection`
        - [ ] `new_node`
        - [x] `new_user`
        - [x] `revoke`
