.. -*- rst -*-

:orphan:

.. highlightlang:: none

News
====

.. _release-8-0-4:

Release 8.0.4 - 2018-06-29
--------------------------

Improvements
^^^^^^^^^^^^

* [log] Add sub error for error message ``[ii][update][one]``.

* Added a new API: ``grn_highlighter_clear_keywords()``.

* Added a new predicate: ``grn_obj_is_number_family_bulk()``.

* Added a new API: ``grn_plugin_proc_get_value_mode()``.

* [:doc:`reference/functions/vector_find`] Added a new function ``vector_find()``.

* Suppress memcpy warnings in msgpack.

* Updated mruby from 1.0.0 to 1.4.1.

* [doc][:doc:`/reference/api/grn_obj`] Added API reference for ``grn_obj_is_index_column()``.

* [windows] Suppress printf format warnings.

* [windows] Suppress warning by msgpack.

* [:doc:`/reference/api/grn_obj`][:doc:`/reference/api/plugin`] Added encoding converter.
  rules:

  * grn_ctx::errbuf: grn_encoding

  * grn_logger_put: grn_encoding

  * mruby: UTF-8

  * path: locale

* [mrb] Added ``LocaleOutput``.

* [windows] Supported converting image path to grn_encoding.

* [:doc:`/reference/tokenizers`][TokenMecab] Convert error message encoding.

* [:doc:`/reference/window_functions/window_sum`] Supported dynamic column as a target column.

* [doc][:doc:`/reference/api/grn_obj`] Added API reference for ``grn_obj_is_vector_column()``.

* [:doc:`/reference/commands/column_create`] Added more validations.

  * 1: Full text search index for vector column must have ``WITH_SECTION`` flag.
    (Note that TokenDelmit with ``WITH_POSITION`` without ``WITH_SECTION`` is permitted.
    It's useful pattern for tag search.)

  * 2: Full text search index for vector column must not be multi column index.
    detail: https://github.com/groonga/groonga/commit/08e2456ba35407e3d5172f71a0200fac2a770142

* [:doc:`/reference/executables/grndb`] Disabled log check temporarily.
  Because it's not completed yet.

Fixes
^^^^^

* [:doc:`reference/functions/sub_filter`] Fixed too much score with a too filtered case.

* Fixed build error if KyTea is installed.

* [:doc:`/reference/executables/grndb`] Fixed output channel.

* [query-log][show-condition] Maybe fixed a crash bug.

* [highlighter][lexicon] Fixed a not highlighted bug.
  The keyword wasn't highlighted if keyword length is less than N ("N"-gram.
  In many cases, it's Bigram so "less than 2").

* [windows] Fixed a base path detection bug.
  If system locale DLL path includes 0x5c (``\`` in ASCII) such as "U+8868
  CJK UNIFIED IDEOGRAPH-8868" in CP932, the base path detection is buggy.

* [:doc:`/reference/tokenizers`][TokenNgram] Fixed wrong first character length.
  It's caused for "PARENTHESIZED IDEOGRAPH" characters such as
  "U+3231 PARENTHESIZED IDEOGRAPH STOCK".

.. _release-8-0-3:

Release 8.0.3 - 2018-05-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/functions/highlight_html`] Support highlight of results of
  the search by ``NormalizerNFKC100`` or ``TokenNgram``.

* [:doc:`/reference/tokenizers`] Added new option for ``TokenNgram`` that
  ``report_source_location option`` .
  This option used when highlighting with ``highlight_html`` use a lexicon.

* [:doc:`/reference/normalizers`] Added new option for ``NormalizerNFKC100`` that
  ``unify_middle_dot option``.
  This option normalizes middle dot. You can search with or without ``・``
  (middle dot) and regardless of ``・`` position.

* [:doc:`/reference/normalizers`] Added new option for ``NormalizerNFKC100`` that
  ``unify_katakana_v_sounds option``.
  This option normalizes ``ヴァヴィヴヴェヴォ`` (katakana) to ``バビブベボ`` (katakana).
  For example, you can search ``バイオリン`` (violin) in ``ヴァイオリン`` (violin).

* [:doc:`/reference/normalizers`] Added new option for ``NormalizerNFKC100`` that
  ``unify_katakana_bu_sound option``.
  This option normalizes ``ヴァヴィヴゥヴェヴォ`` (katakana) to ``ブ`` (katakana).
  For example, you can search ``セーブル`` (katakana) and ``セーヴル`` in
  ``セーヴェル`` (katakana).

* [:doc:`reference/functions/sub_filter`] Supported ``sub_filter`` optimization
  for the too filter case.
  this optimize is valid when records are enough narrowed down before
  ``sub_filter`` execution as below.

* [:doc:`/reference/executables/groonga-httpd`] Made all workers context address
  to unique.
  context address is ``#{ID}`` of below query log.

  | #{TIME_STAMP}|#{MESSAGE}
  | #{TIME_STAMP}|#{ID}|>#{QUERY}
  | #{TIME_STAMP}|#{ID}|:#{ELAPSED_TIME} #{PROGRESS}
  | #{TIME_STAMP}|#{ID}|<#{ELAPSED_TIME} #{RETURN_CODE}

* [:doc:`/reference/commands/delete`] Added new options that ``limit``.
  You can limit the number of delete records as below example.
  ``delete --table Users --filter '_key @^ "b"' --limit 4``

* [httpd] Updated bundled nginx to 1.14.0.

Fixes
^^^^^

* [:doc:`/reference/commands/logical_select`] Fixed memory leak when an error occurs
  in filtered dynamic columns.

* [:doc:`/reference/commands/logical_count`] Fixed memory leak on initial dynamic
  column error.

* [:doc:`/reference/commands/logical_range_filter`] Fixed memory leak when an error
  occurs in dynamic column evaluation.

* [:doc:`/reference/tokenizers`] Fixed a bug that the wrong ``source_offset`` when a
  loose tokenizing such as ``loose_symbol`` option.

* [:doc:`/reference/normalizers`] Fixed a bug that FULLWIDTH LATIN CAPITAL LETTERs
  such as ``U+FF21 FULLWIDTH LATIN CAPITAL LETTER A`` aren't normalized to LATIN SMALL
  LETTERs such as ``U+0061 LATIN SMALL LETTER A``.
  If you have been used ``NormalizerNFKC100`` , you must recreate your indexes.

.. _release-8-0-2:

Release 8.0.2 - 2018-04-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/executables/grndb`][:ref:`grndb-force-truncate`] Improved
  ``grndb recover --force-truncate`` option that it can be truncated even if
  locks are left on the table.

* [:doc:`/reference/commands/logical_range_filter`] Added ``sort_keys`` option.

* Added a new function ``time_format()``.
  You can specify time format against a column of ``Time`` type.
  You can specify with use format of ``strftime`` .

* [:doc:`/reference/tokenizers`] Support new tokenizer ``TokenNgram``.
  You can change its behavior dynamically via options.
  Here is a list of available options:

    * ``n`` : "N" of Ngram. For example, "3" for trigram.
    * ``loose_symbol`` : Tokenize keywords including symbols, to be searched
      by both queries with/without symbols. For example, a keyword
      "090-1111-2222" will be found by any of "09011112222", "090", "1111",
      "2222" and "090-1111-2222".
    * ``loose_blank`` : Tokenize keywords including blanks, to be searched
      by both queries with/without blanks. For example, a keyword
      "090 1111 2222" will be found by any of "09011112222", "090", "1111",
      "2222" and "090 1111 2222".
    * ``remove_blank`` : Tokenize keywords including blanks, to be searched
      by queries without blanks. For example, a keyword "090 1111 2222" will
      be found by any of "09011112222", "090", "1111" or "2222". Note that
      the keyword won't be found by a query including blanks like
      "090 1111 2222".

* [:doc:`/reference/normalizers`] Support new normalizer "NormalizerNFKC100" based on Unicode NFKC (Normalization Form Compatibility Composition) for Unicode 10.0.

* [:doc:`/reference/normalizers`] Support options for "NormalizerNFKC51" and "NormalizerNFKC100" normalizers.
  You can change their behavior dynamically.
  Here is a list of available options:

    * ``unify_kana`` : Same pronounced characters in all of full-width
      Hiragana, full-width Katakana and half-width Katakana are regarded as
      the same character.
    * ``unify_kana_case`` : Large and small versions of same letters in all of
      full-width Hiragana, full-width Katakana and half-width Katakana are
      regarded as the same character.
    * ``unify_kana_voiced_sound_mark`` : Letters with/without voiced sound
      mark and semi voiced sound mark in all of full-width Hiragana,
      full-width Katakana and half-width Katakana are regarded as the same
      character.
    * ``unify_hyphen`` : The characters like hyphen are regarded as the hyphen.
    * ``unify_prolonged_sound_mark`` : The characters like prolonged sound mark
      are regarded as the prolonged sound mark.
    * ``unify_hyphen_and_prolonged_sound_mark`` : The characters like hyphen
      and prolonged sound mark are regarded as the hyphen.

* [:doc:`/reference/commands/dump`] Support output of tokenizer's options and
  normalizer's options. Groonga 8.0.1 and earlier versions cannot import dump
  including options for tokenizers or normalizers generated by Groonga 8.0.2
  or later, and it will occurs error due to unsupported information.

* [:doc:`/reference/commands/schema`] Support output of tokenizer's options and
  normalizer's options. Groonga 8.0.1 and earlier versions cannot import schema
  including options for tokenizers or normalizers generated by Groonga 8.0.2
  or later, and it will occurs error due to unsupported information.

* Supported Ubuntu 18.04 (Bionic Beaver)

Fixes
^^^^^

* Fixed a bug that unexpected record is matched with space only query.
  [groonga-dev,04609][Reported by satouyuzh]

* Fixed a bug that wrong scorer may be used.
  It's caused when multiple scorers are used as below.
  ``--match_columns 'title || scorer_tf_at_most(content, 2.0)'``.

* Fixed a bug that it may also take so much time to change "thread_limit".

Thanks
^^^^^^

* satouyuzh

.. _release-8-0-1:

Release 8.0.1 - 2018-03-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/log`] Show ``filter`` conditions in query log.
  It's disabled by default. To enable it, you need to set an environment
  variable ``GRN_QUERY_LOG_SHOW_CONDITION=yes``.

* Install ``*.pdb`` into the directory where ``*.dll`` and ``*.exe``
  are installed.

* [:doc:`/reference/commands/logical_count`] Support ``filtered``
  stage dynamic columns.

* [:doc:`/reference/commands/logical_count`]
  [:ref:`logical-count-post-filter`] Added a new filter timing.
  It's executed after ``filtered`` stage columns are generated.

* [:doc:`/reference/commands/logical_select`]
  [:ref:`logical-select-post-filter`] Added a new filter timing.
  It's executed after ``filtered`` stage columns are generated.

* Support LZ4/Zstd/zlib compression for vector data.

* Support alias to accessor such as ``_key``.

* [:doc:`/reference/commands/logical_range_filter`] Optimize
  window function for large result set.
  If we find enough matched records, we don't apply window function
  to the remaining windows.

  TODO: Disable this optimization for small result set if its overhead
  is not negligible. The overhead is not evaluated yet.

* [:doc:`/reference/commands/select`] Added ``match_escalation`` parameter.
  You can force to enable match escalation by ``--match_escalation yes``.
  It's stronger than ``--match_escalation_threshold 99999....999``
  because ``--match_escalation yes`` also works with
  ``SOME_CONDITIONS && column @ 'query'``.
  ``--match_escalation_threshold`` isn't used in this case.

  The default is ``--match_escalation auto``. It doesn't change the
  current behavior.

  You can disable match escalation by ``--match_escalation no``.
  It's the same as ``--match_escalation_threshold -1``.

* [httpd] Updated bundled nginx to 1.13.10.

Fixes
^^^^^

* Fixed memory leak that occurs when a prefix query doesn't match any token.
  [GitHub#820][Patch by Naoya Murakami]

* Fixed a bug that a cache for different databases is used when
  multiple databases are opened in the same process.

* Fixed a bug that a wrong index is constructed.
  This occurs only when the source of a column is a vector column and
  ``WITH_SECTION`` isn't specified.

* Fixed a bug that a constant value can overflow or underflow in
  comparison (>,>=,<,<=,==,!=).

Thanks
^^^^^^

* Naoya Murakami

.. _release-8-0-0:

Release 8.0.0 - 2018-02-09
--------------------------

This is a major version up! But It keeps backward compatibility.
You can upgrade to 8.0.0 without rebuilding database.

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/commands/select`] Added ``--drilldown_adjuster`` and
  ``--drilldowns[LABEL].adjuster``.
  You can adjust score against result of drilldown.

* [:ref:`online-index-construction`] Changed environment variable name
  ``GRN_II_REDUCE_EXPIRE_ENABLE`` to ``GRN_II_REDUCE_EXPIRE_THRESHOLD``.

  ``GRN_II_REDUCE_EXPIRE_THRESHOLD=0 == GRN_II_REDUCE_EXPIRE_ENABLE=no``.
  ``GRN_II_REDUCE_EXPIRE_THRESHOLD=-1`` uses
  ``ii->chunk->max_map_seg / 2`` as threshold.
  ``GRN_II_REDUCE_EXPIRE_THRESHOLD > 0`` uses
  ``MIN(ii->chunk->max_map_seg / 2, GRN_II_REDUCE_EXPIRE_THRESHOLD)``
  as threshold.
  ``GRN_II_REDUCE_EXPIRE_THRESHOLD=32`` is the default.

* [:doc:`/reference/functions/between`] Accept ``between()`` without borders.
  If the number of arguments passed to ``between()`` is 3, the 2nd and 3rd
  arguments are handled as the inclusive edges. [GitHub#685]

Fixes
^^^^^

* Fixed a memory leak for normal hash table.
  [GitHub:mroonga/mroonga#190][Reported by fuku1]

* Fix a memory leak for normal array.

* [:doc:`/reference/commands/select`] Stopped to cache when ``output_columns``
  uses not stable function.

* [Windows] Fixed wrong value report on ``WSASend`` error.

Thanks
^^^^^^

* fuku1

.. _release-7-1-1:

Release 7.1.1 - 2018-01-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/ubuntu`] Dropped Ubuntu 17.04 (Zesty Zapus) support.
  It has reached EOL at Jan 13, 2018.

* Added quorum match support.
  You can use quorum match in both script syntax and query syntax.
  [groonga-talk,385][Suggested by 付超群]

  TODO: Add documents for quorum match syntax and link to them.

* Added custom similarity threshold support in script syntax.
  You can use custom similarity threshold in script syntax.

  TODO: Add document for the syntax and link to it.

* [:doc:`/reference/executables/grndb`][:ref:`grndb-force-lock-clear`]
  Added ``--force-lock-clear`` option. With this option, ``grndb``
  forces to clear locks of database, tables and data columns. You can
  use your database again even if locks are remained in database,
  tables and data columns.

  But this option very risky. Normally, you should not use it. If your
  database is broken, your database is still broken. This option just
  ignores locks.

* [:doc:`/reference/commands/load`] Added surrogate pairs support in
  escape syntax. For example, ``\uD83C\uDF7A`` is processed as ``🍺``.

* [Windows] Changed to use sparse file on Windows. It reduces disk
  space and there are no performance demerit.

* [:ref:`online-index-construction`] Added
  ``GRN_II_REDUCE_EXPIRE_THRESHOLD`` environment variable to control
  when memory maps are expired in index column. It's ``-1`` by
  default. It means that expire timing is depends on index column
  size. If index column is smaller, expire timing is more. If index
  column is larger, expire timing is less.

  You can use the previous behavior by ``0``. It means that Groonga
  always tries to expire.

* [:doc:`/reference/commands/logical_range_filter`]
  [:ref:`logical-range-filter-post-filter`] Added a new filter timing.
  It's executed after ``filtered`` stage generated columns are generated.

Fixes
^^^^^

* Reduced resource usage for creating index for reference vector.
  [GitHub#806][Reported by Naoya Murakami]

* [:doc:`/reference/commands/table_create`] Fixed a bug that a table
  is created even when ``token_filters`` is invalid.
  [GitHub#266]

Thanks
^^^^^^

* 付超群

* Naoya Murakami

.. _release-7-1-0:

Release 7.1.0 - 2017-12-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/commands/load`] Improved the ``load``'s
  query-log format.
  Added detail below items in the ``load``'s query-log.

    * outputs number of loaded records.
    * outputs number of error records and columns.
    * outputs number of total records.

* [:doc:`/reference/commands/logical_count`] Improved the
  ``logical_count``'s query-log format.
  Added detail below items in the ``logical_count``'s query-log.

    * outputs number of count.

* [:doc:`/reference/commands/logical_select`] Improve the
  ``logical_select``'s query-log format.
  Added detail below items in the ``logical_select``'s query-log.

    * log N outputs.
    * outputs plain drilldown.
    * outputs labeled drilldown.
    * outputs selected in each shard.
    * use "[...]" for target information.

* [:doc:`/reference/commands/delete`] Improved the ``delete``'s
  query-log format.
  Added detail below items in the ``delete``'s query-log.

    * outputs number of deleted and error records.
    * outputs number of rest number of records.

* [:doc:`/reference/executables/groonga-server-http`] The server
  executed by ``groonga -s`` ensure stopping by C-c.

* Used ``NaN`` and ``Infinity``, ``-Infinity`` instead of Lisp
  representations(``#<nan>`` and  ``#i1/0``, ``#-i1/0``).

* Supported vector for drilldown calc target.

* Partially supported keyword extraction from regexp search.
  It enables ``highlight_html`` and ``snippet_html`` for regexp search.
  [GitHub#787][Reported by takagi01]

* [bulk] Reduced the number of ``realloc()``.
  ``grn_bulk_*()`` API supports it.

  It improves performance for large output case on Windows.
  For example, it causes 100x faster for 100MB over output.

  Because ``realloc()`` is heavy on Windows.

* Enabled ``GRN_II_OVERLAP_TOKEN_SKIP_ENABLE`` only when its value is "yes".

* Deprecated ``GRN_NGRAM_TOKENIZER_REMOVE_BLANK_DISABLE``.
  Use ``GRN_NGRAM_TOKENIZER_REMOVE_BLANK_ENABLE=no`` instead.

* Added new function ``index_column_source_records``.
  It gets source records of index column.[Patch by Naoya Murakami]

* [:doc:`/reference/commands/select`] Supported negative "offset" for "offset + size - limit" >= 0

* Added ``grn_column_cache``.
  It'll improve performance for getter of fixed size column value.

* [:doc:`/reference/executables/groonga`] Added ``--listen-backlog option``.
  You can customize ``listen(2)``'s backlog by this option.

* [httpd] Updated bundled nginx to 1.13.8.

Fixes
^^^^^

* Fixed a memory leak in ``highlight_full``

* Fixed a crash bug by early unlink
  It's not caused by instruction in ``grn_expr_parse()`` but it's caused when
  libgroonga user such as Mroonga uses the following instructions:

    1. ``grn_expr_append_const("_id")``
    2. ``grn_expr_append_op(GRN_OP_GET_VALUE)``

Thanks
^^^^^^

* takagi01
* Naoya Murakami

.. _release-7-0-9:

Release 7.0.9 - 2017-11-29
--------------------------

Improvements
^^^^^^^^^^^^

* Supported newer version of Apache Arrow. In this release, 0.8.0 or
  later is required for Apache Arrow support.

* [sharding] Added new API for dynamic columns.

  * Groonga::LabeledArguments

* [sharding] Added convenient ``Table#select_all`` method.

* [:doc:`/reference/commands/logical_range_filter`] Supported dynamic
  columns. Note that ``initial`` and ``filtered`` stage are only
  supported.

* [:doc:`/reference/commands/logical_range_filter`] Added documentation
  about ``cache`` parameter and dynamic columns.

* [:doc:`/reference/commands/logical_count`] Supported dynamic
  columns. Note that ``initial`` stage is only supported.

* [:doc:`/reference/commands/logical_count`] Added documentation about
  named parameters.

* [:doc:`/reference/commands/select`] Supported ``--match_columns _key``
  without index.

* [:doc:`/reference/functions/in_values`] Supported to specify more
  than 126 values. [GitHub#760] [GitHub#781] [groonga-dev,04449]
  [Reported by Murata Satoshi]

* [httpd] Updated bundled nginx to 1.13.7.

Fixes
^^^^^

* [httpd] Fixed build error when old Groonga is already installed.
  [GitHub#775] [Reported by myamanishi3]

* [:doc:`/reference/functions/in_values`] Fixed a bug that
  ``in_values`` with too many arguments can cause a crash. This bug is
  found during supporting more than 126 values. [GitHub#780]

* [cmake] Fixed LZ4 and MessagePack detection. [Reported by Sergei
  Golubchik]

* [:ref:`offline-index-construction`] Fixed a bug that offline index
  construction for vector column consumes unnecessary resources. If
  you have a log of elements in one vector column and many records,
  Groonga will crash.
  [groonga-dev,04533][Reported by Toshio Uchiyama]

Thanks
^^^^^^

* Murata Satoshi
* myamanishi3
* Sergei Golubchik
* Toshio Uchiyama

.. _release-7-0-8:

Release 7.0.8 - 2017-10-29
--------------------------

Improvements
^^^^^^^^^^^^

* [windows] Supported backtrace on crash.
  This feature not only function call history but also source filename
  and number of lines can be displayed as much as possible.
  This feature makes problem solving easier.

* Supported ``( )`` (empty block) only query (``--query "( )"``) for
  ``QUERY_NO_SYNTAX_ERROR``. In the previous version, it caused an
  error. [GitHub#767]

* Supported ``(+)`` (only and block) only query (``--query "(+)"``)
  for ``QUERY_NO_SYNTAX_ERROR``. In the previous version, it caused an
  error. [GitHub#767]

* Supported ``~foo`` (starting with "~") query (``--query "~y"``) for
  ``QUERY_NO_SYNTAX_ERROR``. In the previous version, it caused an
  error. [GitHub#767]

* Modified log level of ``expired`` from ``info`` to ``debug``.
  ``2017-10-29 14:05:34.123456|i| <0000000012345678:0> expired
  i=000000000B123456 max=10 (2/2)`` This message is logged when memory
  mapped area for index is unmapped.  Thus, this log message is useful
  information for debugging, in other words, as it is unnecessary
  information in normal operation, we changed log level from ``info``
  to ``debug``.

* Supported Ubuntu 17.10 (Artful Aardvark)

Fixes
^^^^^

* [dat] Fixed a bug that large file is created unexpectedly in the
  worst case during database expansion process. This bug may occurs
  when you create/delete index columns so frequently. In 7.0.7
  release, a related bug was fixed - "``table_create`` command fails
  when there are many deleted keys", but it turns out that it is not
  enough in the worst case.

* [:doc:`/reference/commands/logical_select`] Fixed a bug that when
  ``offset`` and ``limit`` were applied to multiple shards at the same
  time, there is a case that it returns a fewer number of records
  unexpectedly.

.. _release-7-0-7:

Release 7.0.7 - 2017-09-29
--------------------------

Improvements
^^^^^^^^^^^^

* Supported ``+`` only query (``--query "+"``) for
  ``QUERY_NO_SYNTAX_ERROR``. In the previous version, it caused an
  error.

* [httpd] Updated bundled nginx to 1.13.5.

* [:doc:`/reference/commands/dump`] Added the default argument values
  to the syntax section.

* [:doc:`/reference/command/command_version`] Supported ``--default-command-version 3``.

* Supported caching select result with function call. Now, most of
  existing functions supports this feature. There are two exception,
  when ``now()`` and ``rand()`` are used in query, select result will
  not cached. Because of this default behavior change, new APIs are
  introduced.

  * ``grn_proc_set_is_stable()``
  * ``grn_proc_is_stable()``

  Note that if you add a new function that may return different result
  with the same argument, you must call ``grn_proc_is_stable(ctx,
  proc, GRN_FALSE)``.  If you don't call it, select result with the
  function call is cached and is wrong result for multiple requests.

Fixes
^^^^^

* [windows] Fixed to clean up file handle correctly on failure when
  ``database_unmap`` is executed. There is a case that critical
  section is not initialized when request is canceled before executing
  ``database_unmap``. In such a case, it caused a crach bug.

* [:doc:`/reference/tokenizers`] Fixed document for wrong tokenizer
  names. It should be ``TokenBigramIgnoreBlankSplitSymbolAlpha`` and
  ``TokenBigramIgnoreBlankSplitSymbolAlphaDigit``.

* Changed not to keep created empty file on error.

  In the previous versions, there is a case that empty file keeps
  remain on error.

  Here is the senario to reproduce:

    1. creating new file by grn_fileinfo_open succeeds
    2. mapping file by DO_MAP() is failed

  In such a case, it causes an another error such as
  "already file exists" because of the file which
  isn't under control. so these file should be removed during
  cleanup process.

* Fixed a bug that Groonga may be crashed when search process is
  executed during executing many updates in a short time.

* [:doc:`/reference/commands/table_create`] Fixed a bug that
  ``table_create`` failed when there are many deleted keys.

.. _release-7-0-6:

Release 7.0.6 - 2017-08-29
--------------------------

Improvements
^^^^^^^^^^^^

* Supported prefix match search using multiple
  indexes. (e.g. ``--query "Foo*" --match_columns
  "TITLE_INDEX_COLUMN||BODY_INDEX_COLUMN"``).

* [:doc:`/reference/window_functions/window_count`] Supported
  ``window_count`` function to add count data to result set. It is
  useful to analyze or filter additionally.

* Added the following API

  * ``grn_obj_get_disk_usage():``
  * ``GRN_EXPR_QUERY_NO_SYNTAX_ERROR``
  * ``grn_expr_syntax_expand_query_by_table()``
  * ``grn_table_find_reference_object()``

* [:doc:`/reference/commands/object_inspect`] Supported to show disk
  usage about specified object.

* Supported falling back query parse feature. It is enabled when
  ``QUERY_NO_SYNTAX_ERROR`` flag is set to ``query_flags``. (this
  feature is disabled by default). If this flag is set, query never
  causes syntax error. For example, "A +" is parsed and escaped
  automatically into "A \+". This behavior is useful when application
  uses user input directly and doesn't want to show syntax error to
  user and in log.

* Supported to adjust score for term in query. ">", "<", and "~"
  operators are supported. For example, ">Groonga" increments score of
  "Groonga", "<Groonga" decrements score of "Groonga". "~Groonga"
  decreases score of matched document in the current search
  result. "~" operator doesn't change search result itself.

* Improved performance to remove table. ``thread_limit=1`` is not
  needed for it. The process about checking referenced table existence
  is done without opening objects. As a result, performance is
  improved.

* [httpd] Updated bundled nginx to 1.13.4.

Fixes
^^^^^

* [:doc:`/reference/commands/dump`] Fixed a bug that the 7-th unnamed
  parameter for `--sort_hash_table` option is ignored.

* [:doc:`/reference/commands/schema`] Fixed a typo in command line
  parameter name. It should be `source` instead of `sources`.
  [groonga-dev,04449] [Reported by murata satoshi]

* [:doc:`/reference/commands/ruby_eval`] Fixed crash when ruby_eval
  returned syntax error. [GitHub#751] [Patch by ryo-pinus]

Thanks
^^^^^^

* murata satoshi

* ryo-pinus

.. _release-7-0-5:

Release 7.0.5 - 2017-07-29
--------------------------

Improvements
^^^^^^^^^^^^

* [httpd] Updated bundled nginx to 1.13.3. Note that this version
  contains security fix for CVE-2017-7529.

* [:doc:`/reference/commands/load`] Supported to load the value of max
  UInt64. In the previous versions, max UInt64 value is converted into
  0 unexpectedlly.

* Added the following API

  * ``grn_window_get_size()`` [GitHub#725] [Patch by Naoya Murakami]

* [:doc:`/reference/functions/math_abs`] Supported ``math_abs()``
  function to calculate absolute value. [GitHub#721]

* Supported to make ``grn_default_logger_set_path()`` and
  ``grn_default_query_logger_set_path()`` thread safe.

* [windows] Updated bundled pcre library to 8.41.

* [:doc:`/reference/commands/normalize`] Improved not to output
  redundant empty string ``""`` on error. [GitHub#730]

* [functions/time] Supported to show error message when division by
  zero was happened. [GitHub#733] [Patch by Naoya Murakami]

* [windows] Changed to map ``ERROR_NO_SYSTEM_RESOURCES`` to
  ``GRN_RESOURCE_TEMPORARILY_UNAVAILABLE``. In the previous versions,
  it returns ``rc=-1`` as a result code. It is not helpful to
  investigate what actually happened. With this fix, it returns
  ``rc=-12``.

* [functions/min][functions/max] Supported vector column. Now you need
  not to care scalar column or vector column to use. [GitHub#735]
  [Patch by Naoya Murakami]

* [:doc:`/reference/commands/dump`] Supported ``--sort_hash_table``
  option to sort by ``_key`` for hash table. Specify
  ``--sort_hash_table yes`` to use it.

* [:doc:`/reference/functions/between`] Supported to specify index
  column. [GitHub#740] [Patch by Naoya Murakami]

* [load] Supported Apache Arrow 0.5.0 or later.

* [:doc:`/troubleshooting/how_to_analyze_error_message`]
  Added howto article to analyze error message in Groonga.

* [:doc:`/install/debian`] Updated required package list to
  build from source.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 16.10 (Yakkety
  Yak) support. It has reached EOL at July 20, 2017.

Fixes
^^^^^

* Fixed to construct correct fulltext indexes against vector column
  which type belongs to text family (```ShortText`` and so on). This
  fix resolves that fulltext search doesn't work well against text
  vector column after updating indexes. [GitHub#494]

* [:doc:`/reference/commands/thread_limit`] Fixed a bug that deadlock
  occurs when thread_limit?max=1 is requested at once.

* [:doc:`/reference/executables/groonga-httpd`] Fixed a mismatch path
  of pid file between default one and restart command assumed. This
  mismatch blocked restarting groonga-httpd. [GitHub#743] [Reported by
  sozaki]

Thanks
^^^^^^

* Naoya Murakami

.. _release-7-0-4:

Release 7.0.4 - 2017-06-29
--------------------------

Improvements
^^^^^^^^^^^^

* Added physical create/delete operation logs to identify problem for
  troubleshooting. [GitHub#700,#701]

* [:doc:`/reference/functions/in_records`] Improved performance for
  fixed sized column. It may reduce 50% execution time.

* [:doc:`/reference/executables/grndb`] Added ``--log-path`` option.
  [GitHub#702,#703]

* [:doc:`/reference/executables/grndb`] Added ``--log-level`` option.
  [GitHub#706,#708]

* Added the following API

  * ``grn_operator_to_exec_func()``
  * ``grn_obj_is_corrupt()``

* Improved performance for "FIXED_SIZE_COLUMN OP CONSTANT". Supported
  operators are: ``==``, ``!=``, ``<``, ``>``, ``<=`` and ``>=``.

* Improved performance for "COLUMN OP VALUE && COLUMN OP VALUE && ...".

* [:doc:`/reference/executables/grndb`] Supported corrupted object
  detection with ``grndb check``.

* [:doc:`/reference/commands/io_flush`] Supported ``--only_opened``
  option which enables to flush only opened database objects.

* [:doc:`/reference/executables/grndb`] Supported to detect/delete
  orphan "inspect" object. The orphaned "inspect" object is created by
  renamed command name from ``inspect`` to ``object_inspect``.

Fixes
^^^^^

* [rpm][centos] Fixed unexpected macro expansion problem with
  customized build. This bug only affects when rebuilding Groonga SRPM
  with customized ``additional_configure_options`` parameter in spec
  file.

* Fixed missing null check for ``grn_table_setoperation()``. There is a
  possibility of crash bug when indexes are broken. [GitHub#699]

Thanks
^^^^^^

.. _release-7-0-3:

Release 7.0.3 - 2017-05-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/commands/select`] Add document about
  :ref:`full-text-search-with-specific-index-name`.

* [index] Supported to log warning message which record causes posting
  list overflows.

* [:doc:`/reference/commands/load`][:doc:`/reference/commands/dump`]
  Supported Apache Arrow. [GitHub#691]

* [cmake] Supported linking lz4 in embedded static library build.
  [Original patch by Sergei Golubchik]

* [:doc:`/reference/commands/delete`] Supported to cancel.

* [httpd] Updated bundled nginx to 1.13.0

* Exported the following API

  * grn_plugin_proc_get_caller()

* Added index column related function and selector.

  * Added new selector: index_column_df_ratio_between()

  * Added new function: index_column_df_ratio()

Fixes
^^^^^

* [:doc:`/reference/commands/delete`] Fixed a bug that error isn't
  cleared correctly. It affects to following deletions so that it
  causes unexpected behavior.

* [windows] Fixed a bug that IO version is not detected correctly when the
  file is opened with ``O_CREAT`` flag.

* [:doc:`/reference/functions/vector_slice`] Fixed a bug that non 4
  bytes vector columns can't slice. [GitHub#695] [Patch by Naoya
  Murakami]

* Fixed a bug that non 4 bytes fixed vector column can't sequential
  match by specifying index of vector. [GitHub#696] [Patch by Naoya
  Murakami]

* [:doc:`/reference/commands/logical_select`] Fixed a bug that
  "argument out of range" occurs when setting last day of month to the
  min. [GitHub#698]

Thanks
^^^^^^

* Sergei Golubchik

* Naoya Murakami

.. _release-7-0-2:

Release 7.0.2 - 2017-04-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/commands/logical_select`] Supported multiple
  :ref:`logical-select-drilldowns-label-columns-name-window-sort-keys`
  and
  :ref:`logical-select-drilldowns-label-columns-name-window-group-keys`.

* [windows] Updated bundled LZ4 to 1.7.5.

* [cache] Supported persistent cache feature.

* [:doc:`/reference/commands/log_level`] Update English documentation.

* Added the following APIs:

  * ``grn_set_default_cache_base_path()``
  * ``grn_get_default_cache_base_path()``
  * ``grn_persistent_cache_open()``
  * ``grn_cache_default_open()``

* [:option:`groonga --cache-base-path`] Added a new option to use
  persistent cache.

* [:doc:`/reference/executables/groonga-httpd`]
  [:ref:`groonga-httpd-groonga-cache-base-path`] Added new
  configuration to use persistent cache.

* [windows] Updated bundled msgpack to 2.1.1.

* [:doc:`/reference/commands/object_inspect`] Supported not only
  column inspection, but also index column statistics.

* Supported index search for "``.*``" regexp pattern.  This feature is
  enabled by default. Set
  ``GRN_SCAN_INFO_REGEXP_DOT_ASTERISK_ENABLE=no`` environment variable
  to disable this feature.

* [:doc:`/reference/functions/in_records`] Added function to use an
  existing table as condition patterns.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 12.04 (Precise Pangolin)
  support because of EOL.

Fixes
^^^^^

* [:doc:`/reference/commands/logical_select`] Fixed a bug that wrong
  cache is used. This bug was occurred when dynamic column parameter
  is used.

* [:doc:`/reference/commands/logical_select`] Fixed a bug that dynamic
  columns aren't created. It's occurred when no match case.

* [:doc:`/reference/commands/reindex`] Fixed a bug that data is lost
  by reindex. [GitHub#646]

* [httpd] Fixed a bug that response of :doc:`/reference/commands/quit`
  and :doc:`/reference/commands/shutdown` is broken JSON when worker is
  running as another user. [GitHub ranguba/groonga-client#12]

.. _release-7-0-1:

Release 7.0.1 - 2017-03-29
--------------------------

Improvements
^^^^^^^^^^^^

* Exported the following API

  * grn_ii_cursor_next_pos()
  * grn_table_apply_expr()
  * grn_obj_is_data_column()
  * grn_obj_is_expr()
  * grn_obj_is_scalar_column()

* [:doc:`/reference/commands/dump`] Supported to dump weight reference
  vector.

* [:doc:`/reference/commands/load`] Supported to load
  ``array<object>`` style weight vector column. The example of
  ``array<object>`` style is: ``[{"key1": weight1}, {"key2":
  weight2}]``.

* Supported to search ``!(XXX OPERATOR VALUE)`` by index. Supported
  operator is not only ``>`` but also ``>=``, ``<``, ``<=``, ``==``
  and ``!=``.

* Supported index search for "!(column == CONSTANT)". The example in
  this case is: ``!(column == 29)`` and so on.

* Supported more "!" optimization in the following patterns.

  * ``!(column @ "X") && (column @ "Y")``
  * ``(column @ "Y") && !(column @ "X")``
  * ``(column @ "Y") &! !(column @ "X")``

* Supported to search ``XXX || !(column @ "xxx")`` by index.

* [:doc:`/reference/commands/dump`] Changed to use ``'{"x": 1, "y":
  2}'`` style for not referenced weight vector. This change doesn't
  affect to old Groonga because it already supports one.

* [experimental] Supported ``GRN_ORDER_BY_ESTIMATED_SIZE_ENABLE``
  environment variable. This variable controls whether query
  optimization which is based on estimated size is applied or not.
  This feature is disabled by default. Set
  ``GRN_ORDER_BY_ESTIMATED_SIZE_ENABLE=yes`` if you want to try it.

* [:doc:`/reference/commands/select`] Added query log for ``columns``,
  ``drilldown`` evaluation.

* [:doc:`/reference/commands/select`] Changed query log format for
  ``drilldown``. This is backward incompatible change, but it only
  affects users who convert query log by own programs.

* [:doc:`/reference/commands/table_remove`] Reduced temporary memory
  usage. It's enabled when the number of max threads is 0.

* [:doc:`/reference/commands/select`] ``columns[LABEL](N)`` is used
  for query log format instead of ``columns(N)[LABEL]``..

* [:doc:`/tutorial/query_expansion`] Updated example to use vector
  column because it is recommended way. [Reported by Gurunavi, Inc]

* Supported to detect canceled request while locking. It fixes the
  problem that ``request_cancel`` is ignored unexpectedly while locking.

* [:doc:`/reference/commands/logical_select`] Supported ``initial``
  and ``filtered`` stage dynamic columns. The examples are:
  ``--columns[LABEL].stage initial`` or ``--columns[LABEL].stage
  filtered``.

* [:doc:`/reference/commands/logical_select`] Supported
  ``match_columns``, ``query`` and ``drilldown_filter`` option.

* [:doc:`/reference/functions/highlight_html`] Supported similar
  search.

* [:doc:`/reference/commands/logical_select`] Supported ``initial`` 
  and stage dynamic columns in labeled drilldown. The example is: 
  ``--drilldowns[LABEL].stage initial``.

* [:doc:`/reference/commands/logical_select`] Supported window
  function in dynamic column.

* [:doc:`/reference/commands/select`] Added documentation about
  dynamic columns.

* [:doc:`/reference/window_function`] Added section about window
  functions.

* [:doc:`/install/centos`] Dropped CentOS 5 support because of EOL.

* [httpd] Updated bundled nginx to 1.11.12

* Supported to disable AND match optimization by environment variable.
  You can disable this feature by
  ``GRN_TABLE_SELECT_AND_MIN_SKIP_ENABLE=no``. This feature is enable
  by default.

* [:doc:`/reference/functions/vector_new`] Added a new function to
  create a new vector.

* [:doc:`/reference/commands/select`] Added documentation about
  ``drilldown_filter``.

Fixes
^^^^^

* [:doc:`/reference/commands/lock_clear`] Fixed a crash bug against
  temporary database.

* Fixed a problem that dynamically updated index size was increased
  for natural language since Grooonga 6.1.4.

* [:doc:`/reference/commands/select`] Fixed a bug that "A && B.C @ X"
  may not return records that should be matched.

* Fixed a conflict with ``grn_io_flush()`` and
  ``grn_io_expire()``. Without this change, if ``io_flush`` and ``load``
  command are executed simultaneously in specific timing, it causes a
  crash bug by access violation.

* [:doc:`/reference/commands/logical_table_remove`] Fixed a crash bug
  when the max number of threads is 1.

Thanks
^^^^^^

* Gurunavi, Inc.
  
.. _release-7-0-0:

Release 7.0.0 - 2017-02-09
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/functions/in_values`] Supported sequential search
  for reference vector column. [Patch by Naoya Murakami] [GitHub#629]

* [:doc:`/reference/commands/select`] Changed to report error instead
  of ignoring on invalid ``drilldown[LABEL].sort_keys``.

* [:doc:`/reference/commands/select`] Removed needless metadata
  updates on DB. It reduces the case that database lock remains
  even though ``select`` command is executed. [Reported by aomi-n]

* [:doc:`/reference/commands/lock_clear`] Changed to clear metadata lock
  by lock_clear against DB.

* [:doc:`/install/centos`] Enabled EPEL by default to install Groonga
  on Amazon Linux.

* [:doc:`/reference/functions/query`] Supported "@X" style in script
  syntax for prefix("@^"), suffix("@$"), regexp("@^") search.

* [:doc:`/reference/functions/query`] Added documentation about
  available list of mode. The default mode is ``MATCH`` ("@") mode
  which executes full text search.

* [rpm][centos] Supported groonga-token-filter-stem package which
  provides stemming feature by ``TokenFilterStem`` token filter on
  CentOS 7. [GitHub#633] [Reported by Tim Bellefleur]

* [:doc:`/reference/window_functions/window_record_number`] Marked
  ``record_number`` as deprecated. Use ``window_record_number``
  instead. ``record_number`` is still available for backward
  compatibility.

* [:doc:`/reference/window_functions/window_sum`] Added ``window_sum``
  window function. It's similar behavior to window function sum() on
  PostgreSQL.

* Supported to construct offline indexing with in-memory (temporary)
  ``TABLE_DAT_KEY`` table. [GitHub#623] [Reported by Naoya Murakami]

* [onigmo] Updated bundled Onigmo to 6.1.1.

* Supported ``columns[LABEL].window.group_keys``. It's used to apply
  window function for every group.

* [:doc:`/reference/commands/load`] Supported to report error on
  invalid key. It enables you to detect mismatch type of key.

* [:doc:`/reference/commands/load`] Supported ``--output_errors yes``
  option. If you specify "yes", you can get errors for each load
  failed record. Note that this feature requires command version 3.

* [:doc:`/reference/commands/load`] Improve error message on table key
  cast failure. Instead of "cast failed", type of table key and target
  type of table key are also contained in error message.

* [httpd] Updated bundled nginx to 1.11.9.

Fixes
^^^^^

* Fixed a bug that nonexistent sort keys for ``drilldowns[LABEL]`` or
  ``slices[LABEL]`` causes invalid JSON parse error. [Patch by Naoya
  Murakami] [GitHub#627]

* Fixed a bug that access to nonexistent sub records for group causes
  a crash.  For example, This bug affects the case when you use
  ``drilldowns[LABEL].sort_keys _sum`` without specifying
  ``calc_types``.  [Patch by Naoya Murakami] [GitHub#625]

* Fixed a crash bug when tokenizer has an error. It's caused when
  tokenizer and token filter are registered and tokenizer has an
  error.

* [:doc:`/reference/window_functions/window_record_number`] Fixed a
  bug that arguments for window function is not correctly
  passed. [GitHub#634][Patch by Naoya Murakami]

Thanks
^^^^^^

* Naoya Murakami
* aomi-n

The old releases
----------------

.. toctree::
   :maxdepth: 2

   news/6.x
   news/5.x
   news/4.x
   news/3.x
   news/2.x
   news/1.3.x
   news/1.2.x
   news/1.1.x
   news/1.0.x
   news/0.x
   news/senna
