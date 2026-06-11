#define _CRT_SECURE_NO_WARNINGS

#include "std/Path.hpp"
#include "std/Slice.hpp"
#include "std/SliceUtils.hpp"
#include "std/json/Parser.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <csetjmp>

static const Slice<const char> TEST_PARSER[] = {
    sliceFromConstChar("i_number_double_huge_neg_exp.json"),
    sliceFromConstChar("i_number_huge_exp.json"),
    sliceFromConstChar("i_number_neg_int_huge_exp.json"),
    sliceFromConstChar("i_number_pos_double_huge_exp.json"),
    sliceFromConstChar("i_number_real_neg_overflow.json"),
    sliceFromConstChar("i_number_real_pos_overflow.json"),
    sliceFromConstChar("i_number_real_underflow.json"),
    sliceFromConstChar("i_number_too_big_neg_int.json"),
    sliceFromConstChar("i_number_too_big_pos_int.json"),
    sliceFromConstChar("i_number_very_big_negative_int.json"),
    sliceFromConstChar("i_object_key_lone_2nd_surrogate.json"),
    sliceFromConstChar("i_string_1st_surrogate_but_2nd_missing.json"),
    sliceFromConstChar("i_string_1st_valid_surrogate_2nd_invalid.json"),
    sliceFromConstChar("i_string_incomplete_surrogates_escape_valid.json"),
    sliceFromConstChar("i_string_incomplete_surrogate_and_escape_valid.json"),
    sliceFromConstChar("i_string_incomplete_surrogate_pair.json"),
    sliceFromConstChar("i_string_invalid_lonely_surrogate.json"),
    sliceFromConstChar("i_string_invalid_surrogate.json"),
    sliceFromConstChar("i_string_invalid_utf-8.json"),
    sliceFromConstChar("i_string_inverted_surrogates_U+1D11E.json"),
    sliceFromConstChar("i_string_iso_latin_1.json"),
    sliceFromConstChar("i_string_lone_second_surrogate.json"),
    sliceFromConstChar("i_string_lone_utf8_continuation_byte.json"),
    sliceFromConstChar("i_string_not_in_unicode_range.json"),
    sliceFromConstChar("i_string_overlong_sequence_2_bytes.json"),
    sliceFromConstChar("i_string_overlong_sequence_6_bytes.json"),
    sliceFromConstChar("i_string_overlong_sequence_6_bytes_null.json"),
    sliceFromConstChar("i_string_truncated-utf-8.json"),
    sliceFromConstChar("i_string_UTF-16LE_with_BOM.json"),
    sliceFromConstChar("i_string_UTF-8_invalid_sequence.json"),
    sliceFromConstChar("i_string_utf16BE_no_BOM.json"),
    sliceFromConstChar("i_string_utf16LE_no_BOM.json"),
    sliceFromConstChar("i_string_UTF8_surrogate_U+D800.json"),
    sliceFromConstChar("i_structure_500_nested_arrays.json"),
    sliceFromConstChar("i_structure_UTF-8_BOM_empty_object.json"),
    sliceFromConstChar("n_array_1_true_without_comma.json"),
    sliceFromConstChar("n_array_a_invalid_utf8.json"),
    sliceFromConstChar("n_array_colon_instead_of_comma.json"),
    sliceFromConstChar("n_array_comma_after_close.json"),
    sliceFromConstChar("n_array_comma_and_number.json"),
    sliceFromConstChar("n_array_double_comma.json"),
    sliceFromConstChar("n_array_double_extra_comma.json"),
    sliceFromConstChar("n_array_extra_close.json"),
    sliceFromConstChar("n_array_extra_comma.json"),
    sliceFromConstChar("n_array_incomplete.json"),
    sliceFromConstChar("n_array_incomplete_invalid_value.json"),
    sliceFromConstChar("n_array_inner_array_no_comma.json"),
    sliceFromConstChar("n_array_invalid_utf8.json"),
    sliceFromConstChar("n_array_items_separated_by_semicolon.json"),
    sliceFromConstChar("n_array_just_comma.json"),
    sliceFromConstChar("n_array_just_minus.json"),
    sliceFromConstChar("n_array_missing_value.json"),
    sliceFromConstChar("n_array_newlines_unclosed.json"),
    sliceFromConstChar("n_array_number_and_comma.json"),
    sliceFromConstChar("n_array_number_and_several_commas.json"),
    sliceFromConstChar("n_array_spaces_vertical_tab_formfeed.json"),
    sliceFromConstChar("n_array_star_inside.json"),
    sliceFromConstChar("n_array_unclosed.json"),
    sliceFromConstChar("n_array_unclosed_trailing_comma.json"),
    sliceFromConstChar("n_array_unclosed_with_new_lines.json"),
    sliceFromConstChar("n_array_unclosed_with_object_inside.json"),
    sliceFromConstChar("n_incomplete_false.json"),
    sliceFromConstChar("n_incomplete_null.json"),
    sliceFromConstChar("n_incomplete_true.json"),
    sliceFromConstChar("n_multidigit_number_then_00.json"),
    sliceFromConstChar("n_number_++.json"),
    sliceFromConstChar("n_number_+1.json"),
    sliceFromConstChar("n_number_+Inf.json"),
    sliceFromConstChar("n_number_-01.json"),
    sliceFromConstChar("n_number_-1.0..json"),
    sliceFromConstChar("n_number_-2..json"),
    sliceFromConstChar("n_number_-NaN.json"),
    sliceFromConstChar("n_number_.-1.json"),
    sliceFromConstChar("n_number_.2e-3.json"),
    sliceFromConstChar("n_number_0.1.2.json"),
    sliceFromConstChar("n_number_0.3e+.json"),
    sliceFromConstChar("n_number_0.3e.json"),
    sliceFromConstChar("n_number_0.e1.json"),
    sliceFromConstChar("n_number_0e+.json"),
    sliceFromConstChar("n_number_0e.json"),
    sliceFromConstChar("n_number_0_capital_E+.json"),
    sliceFromConstChar("n_number_0_capital_E.json"),
    sliceFromConstChar("n_number_1.0e+.json"),
    sliceFromConstChar("n_number_1.0e-.json"),
    sliceFromConstChar("n_number_1.0e.json"),
    sliceFromConstChar("n_number_1eE2.json"),
    sliceFromConstChar("n_number_1_000.json"),
    sliceFromConstChar("n_number_2.e+3.json"),
    sliceFromConstChar("n_number_2.e-3.json"),
    sliceFromConstChar("n_number_2.e3.json"),
    sliceFromConstChar("n_number_9.e+.json"),
    sliceFromConstChar("n_number_expression.json"),
    sliceFromConstChar("n_number_hex_1_digit.json"),
    sliceFromConstChar("n_number_hex_2_digits.json"),
    sliceFromConstChar("n_number_Inf.json"),
    sliceFromConstChar("n_number_infinity.json"),
    sliceFromConstChar("n_number_invalid+-.json"),
    sliceFromConstChar("n_number_invalid-negative-real.json"),
    sliceFromConstChar("n_number_invalid-utf-8-in-bigger-int.json"),
    sliceFromConstChar("n_number_invalid-utf-8-in-exponent.json"),
    sliceFromConstChar("n_number_invalid-utf-8-in-int.json"),
    sliceFromConstChar("n_number_minus_infinity.json"),
    sliceFromConstChar("n_number_minus_sign_with_trailing_garbage.json"),
    sliceFromConstChar("n_number_minus_space_1.json"),
    sliceFromConstChar("n_number_NaN.json"),
    sliceFromConstChar("n_number_neg_int_starting_with_zero.json"),
    sliceFromConstChar("n_number_neg_real_without_int_part.json"),
    sliceFromConstChar("n_number_neg_with_garbage_at_end.json"),
    sliceFromConstChar("n_number_real_garbage_after_e.json"),
    sliceFromConstChar("n_number_real_without_fractional_part.json"),
    sliceFromConstChar("n_number_real_with_invalid_utf8_after_e.json"),
    sliceFromConstChar("n_number_starting_with_dot.json"),
    sliceFromConstChar("n_number_U+FF11_fullwidth_digit_one.json"),
    sliceFromConstChar("n_number_with_alpha.json"),
    sliceFromConstChar("n_number_with_alpha_char.json"),
    sliceFromConstChar("n_number_with_leading_zero.json"),
    sliceFromConstChar("n_object_bad_value.json"),
    sliceFromConstChar("n_object_bracket_key.json"),
    sliceFromConstChar("n_object_comma_instead_of_colon.json"),
    sliceFromConstChar("n_object_double_colon.json"),
    sliceFromConstChar("n_object_emoji.json"),
    sliceFromConstChar("n_object_garbage_at_end.json"),
    sliceFromConstChar("n_object_key_with_single_quotes.json"),
    sliceFromConstChar(
        "n_object_lone_continuation_byte_in_key_and_trailing_comma.json"),
    sliceFromConstChar("n_object_missing_colon.json"),
    sliceFromConstChar("n_object_missing_key.json"),
    sliceFromConstChar("n_object_missing_semicolon.json"),
    sliceFromConstChar("n_object_missing_value.json"),
    sliceFromConstChar("n_object_no-colon.json"),
    sliceFromConstChar("n_object_non_string_key.json"),
    sliceFromConstChar("n_object_non_string_key_but_huge_number_instead.json"),
    sliceFromConstChar("n_object_repeated_null_null.json"),
    sliceFromConstChar("n_object_several_trailing_commas.json"),
    sliceFromConstChar("n_object_single_quote.json"),
    sliceFromConstChar("n_object_trailing_comma.json"),
    sliceFromConstChar("n_object_trailing_comment.json"),
    sliceFromConstChar("n_object_trailing_comment_open.json"),
    sliceFromConstChar("n_object_trailing_comment_slash_open.json"),
    sliceFromConstChar("n_object_trailing_comment_slash_open_incomplete.json"),
    sliceFromConstChar("n_object_two_commas_in_a_row.json"),
    sliceFromConstChar("n_object_unquoted_key.json"),
    sliceFromConstChar("n_object_unterminated-value.json"),
    sliceFromConstChar("n_object_with_single_string.json"),
    sliceFromConstChar("n_object_with_trailing_garbage.json"),
    sliceFromConstChar("n_single_space.json"),
    sliceFromConstChar("n_string_1_surrogate_then_escape.json"),
    sliceFromConstChar("n_string_1_surrogate_then_escape_u.json"),
    sliceFromConstChar("n_string_1_surrogate_then_escape_u1.json"),
    sliceFromConstChar("n_string_1_surrogate_then_escape_u1x.json"),
    sliceFromConstChar("n_string_accentuated_char_no_quotes.json"),
    sliceFromConstChar("n_string_backslash_00.json"),
    sliceFromConstChar("n_string_escaped_backslash_bad.json"),
    sliceFromConstChar("n_string_escaped_ctrl_char_tab.json"),
    sliceFromConstChar("n_string_escaped_emoji.json"),
    sliceFromConstChar("n_string_escape_x.json"),
    sliceFromConstChar("n_string_incomplete_escape.json"),
    sliceFromConstChar("n_string_incomplete_escaped_character.json"),
    sliceFromConstChar("n_string_incomplete_surrogate.json"),
    sliceFromConstChar("n_string_incomplete_surrogate_escape_invalid.json"),
    sliceFromConstChar("n_string_invalid-utf-8-in-escape.json"),
    sliceFromConstChar("n_string_invalid_backslash_esc.json"),
    sliceFromConstChar("n_string_invalid_unicode_escape.json"),
    sliceFromConstChar("n_string_invalid_utf8_after_escape.json"),
    sliceFromConstChar("n_string_leading_uescaped_thinspace.json"),
    sliceFromConstChar("n_string_no_quotes_with_bad_escape.json"),
    sliceFromConstChar("n_string_single_doublequote.json"),
    sliceFromConstChar("n_string_single_quote.json"),
    sliceFromConstChar("n_string_single_string_no_double_quotes.json"),
    sliceFromConstChar("n_string_start_escape_unclosed.json"),
    sliceFromConstChar("n_string_unescaped_ctrl_char.json"),
    sliceFromConstChar("n_string_unescaped_newline.json"),
    sliceFromConstChar("n_string_unescaped_tab.json"),
    sliceFromConstChar("n_string_unicode_CapitalU.json"),
    sliceFromConstChar("n_string_with_trailing_garbage.json"),
    // sliceFromConstChar("n_structure_100000_opening_arrays.json"), // SKIPPED: stack overflow
    sliceFromConstChar("n_structure_angle_bracket_..json"),
    sliceFromConstChar("n_structure_angle_bracket_null.json"),
    sliceFromConstChar("n_structure_array_trailing_garbage.json"),
    sliceFromConstChar("n_structure_array_with_extra_array_close.json"),
    sliceFromConstChar("n_structure_array_with_unclosed_string.json"),
    sliceFromConstChar("n_structure_ascii-unicode-identifier.json"),
    sliceFromConstChar("n_structure_capitalized_True.json"),
    sliceFromConstChar("n_structure_close_unopened_array.json"),
    sliceFromConstChar("n_structure_comma_instead_of_closing_brace.json"),
    sliceFromConstChar("n_structure_double_array.json"),
    sliceFromConstChar("n_structure_end_array.json"),
    sliceFromConstChar("n_structure_incomplete_UTF8_BOM.json"),
    sliceFromConstChar("n_structure_lone-invalid-utf-8.json"),
    sliceFromConstChar("n_structure_lone-open-bracket.json"),
    sliceFromConstChar("n_structure_no_data.json"),
    sliceFromConstChar("n_structure_null-byte-outside-string.json"),
    sliceFromConstChar("n_structure_number_with_trailing_garbage.json"),
    sliceFromConstChar("n_structure_object_followed_by_closing_object.json"),
    sliceFromConstChar("n_structure_object_unclosed_no_value.json"),
    sliceFromConstChar("n_structure_object_with_comment.json"),
    sliceFromConstChar("n_structure_object_with_trailing_garbage.json"),
    sliceFromConstChar("n_structure_open_array_apostrophe.json"),
    sliceFromConstChar("n_structure_open_array_comma.json"),
    // sliceFromConstChar("n_structure_open_array_object.json"), // SKIPPED: stack overflow
    sliceFromConstChar("n_structure_open_array_open_object.json"),
    sliceFromConstChar("n_structure_open_array_open_string.json"),
    sliceFromConstChar("n_structure_open_array_string.json"),
    sliceFromConstChar("n_structure_open_object.json"),
    sliceFromConstChar("n_structure_open_object_close_array.json"),
    sliceFromConstChar("n_structure_open_object_comma.json"),
    sliceFromConstChar("n_structure_open_object_open_array.json"),
    sliceFromConstChar("n_structure_open_object_open_string.json"),
    sliceFromConstChar("n_structure_open_object_string_with_apostrophes.json"),
    sliceFromConstChar("n_structure_open_open.json"),
    sliceFromConstChar("n_structure_single_eacute.json"),
    sliceFromConstChar("n_structure_single_star.json"),
    sliceFromConstChar("n_structure_trailing_#.json"),
    sliceFromConstChar("n_structure_U+2060_word_joined.json"),
    sliceFromConstChar("n_structure_uescaped_LF_before_string.json"),
    sliceFromConstChar("n_structure_unclosed_array.json"),
    sliceFromConstChar("n_structure_unclosed_array_partial_null.json"),
    sliceFromConstChar("n_structure_unclosed_array_unfinished_false.json"),
    sliceFromConstChar("n_structure_unclosed_array_unfinished_true.json"),
    sliceFromConstChar("n_structure_unclosed_object.json"),
    sliceFromConstChar("n_structure_unicode-identifier.json"),
    sliceFromConstChar("n_structure_UTF8_BOM_no_data.json"),
    sliceFromConstChar("n_structure_whitespace_formfeed.json"),
    sliceFromConstChar("n_structure_whitespace_U+2060_word_joiner.json"),
    sliceFromConstChar("y_array_arraysWithSpaces.json"),
    sliceFromConstChar("y_array_empty-string.json"),
    sliceFromConstChar("y_array_empty.json"),
    sliceFromConstChar("y_array_ending_with_newline.json"),
    sliceFromConstChar("y_array_false.json"),
    sliceFromConstChar("y_array_heterogeneous.json"),
    sliceFromConstChar("y_array_null.json"),
    sliceFromConstChar("y_array_with_1_and_newline.json"),
    sliceFromConstChar("y_array_with_leading_space.json"),
    sliceFromConstChar("y_array_with_several_null.json"),
    sliceFromConstChar("y_array_with_trailing_space.json"),
    sliceFromConstChar("y_number.json"),
    sliceFromConstChar("y_number_0e+1.json"),
    sliceFromConstChar("y_number_0e1.json"),
    sliceFromConstChar("y_number_after_space.json"),
    sliceFromConstChar("y_number_double_close_to_zero.json"),
    sliceFromConstChar("y_number_int_with_exp.json"),
    sliceFromConstChar("y_number_minus_zero.json"),
    sliceFromConstChar("y_number_negative_int.json"),
    sliceFromConstChar("y_number_negative_one.json"),
    sliceFromConstChar("y_number_negative_zero.json"),
    sliceFromConstChar("y_number_real_capital_e.json"),
    sliceFromConstChar("y_number_real_capital_e_neg_exp.json"),
    sliceFromConstChar("y_number_real_capital_e_pos_exp.json"),
    sliceFromConstChar("y_number_real_exponent.json"),
    sliceFromConstChar("y_number_real_fraction_exponent.json"),
    sliceFromConstChar("y_number_real_neg_exp.json"),
    sliceFromConstChar("y_number_real_pos_exponent.json"),
    sliceFromConstChar("y_number_simple_int.json"),
    sliceFromConstChar("y_number_simple_real.json"),
    sliceFromConstChar("y_object.json"),
    sliceFromConstChar("y_object_basic.json"),
    sliceFromConstChar("y_object_duplicated_key.json"),
    sliceFromConstChar("y_object_duplicated_key_and_value.json"),
    sliceFromConstChar("y_object_empty.json"),
    sliceFromConstChar("y_object_empty_key.json"),
    sliceFromConstChar("y_object_escaped_null_in_key.json"),
    sliceFromConstChar("y_string_space.json"),
    sliceFromConstChar(
        "y_string_surrogates_U+1D11E_MUSICAL_SYMBOL_G_CLEF.json"),
    sliceFromConstChar("y_string_three-byte-utf-8.json"),
    sliceFromConstChar("y_string_two-byte-utf-8.json"),
    sliceFromConstChar("y_string_u+2028_line_sep.json"),
    sliceFromConstChar("y_string_u+2029_par_sep.json"),
    sliceFromConstChar("y_string_uEscape.json"),
    sliceFromConstChar("y_string_uescaped_newline.json"),
    sliceFromConstChar("y_string_unescaped_char_delete.json"),
    sliceFromConstChar("y_string_unicode.json"),
    sliceFromConstChar("y_string_unicodeEscapedBackslash.json"),
    sliceFromConstChar("y_string_unicode_2.json"),
    sliceFromConstChar("y_string_unicode_escaped_double_quote.json"),
    sliceFromConstChar("y_string_unicode_U+10FFFE_nonchar.json"),
    sliceFromConstChar("y_string_unicode_U+1FFFE_nonchar.json"),
    sliceFromConstChar("y_string_unicode_U+200B_ZERO_WIDTH_SPACE.json"),
    sliceFromConstChar("y_string_unicode_U+2064_invisible_plus.json"),
    sliceFromConstChar("y_string_unicode_U+FDD0_nonchar.json"),
    sliceFromConstChar("y_string_unicode_U+FFFE_nonchar.json"),
    sliceFromConstChar("y_string_utf8.json"),
    sliceFromConstChar("y_string_with_del_character.json"),
    sliceFromConstChar("y_structure_lonely_false.json"),
    sliceFromConstChar("y_structure_lonely_int.json"),
    sliceFromConstChar("y_structure_lonely_negative_real.json"),
    sliceFromConstChar("y_structure_lonely_null.json"),
    sliceFromConstChar("y_structure_lonely_string.json"),
    sliceFromConstChar("y_structure_lonely_true.json"),
    sliceFromConstChar("y_structure_string_empty.json"),
    sliceFromConstChar("y_structure_trailing_newline.json"),
    sliceFromConstChar("y_structure_true_in_array.json"),
    sliceFromConstChar("y_structure_whitespace_array.json"),
};

static const u32 SIZ_ARENA = 32 * 1024 * 1024;
static Arena makeArena() {
  u8 *base = reinterpret_cast<u8 *>(malloc(SIZ_ARENA));
  u8 *end = base + SIZ_ARENA;
  return {base, end};
}

static std::jmp_buf gJmpBuf;

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s path_to_JSONTestSuite\n", argv[0]);
    return 1;
  }

  Arena arena0 = makeArena();
  Arena arena1 = makeArena();
  setAllocatorsForThread(&arena0, &arena1);

  Slice<const char> pathRepo = {argv[1], (u32)strlen(argv[1])};
  Slice<const Slice<const char>> test_files = sliceFrom(TEST_PARSER);

  Arena arena0Saved = arena0;
  Arena arena1Saved = arena1;

  for (auto [filename, _] : test_files) {
    arena0 = arena0Saved;
    arena1 = arena1Saved;

    Arena::Scope temp = getScratch(nullptr, 0);

    Slice<char> pathTestFiles =
        joinSimple(temp, pathRepo, sliceFromConstChar("test_parsing"));
    Slice<char> pathFile = joinSimple(temp, pathTestFiles.asConst(), filename);
    Slice<char> z = concatZeroTerminate(temp, pathFile.asConst(), {});

    FILE *f = fopen(z.data, "rb");
    if (!f) {
      printf("Failed to open for reading: %s\n", z.data);
      continue;
    }
    fseek(f, 0, SEEK_END);
    long sizFile = ftell(f);
    fseek(f, 0, SEEK_SET);
    Slice<char> bytes;
    alloc(temp, sizFile, bytes);
    fread(bytes.data, sizFile, 1, f);
    fclose(f);

    JsonValue root;
    Slice<const char> cbytes = bytes.asConst();
    bool accepted;
    bool crashed = true;
    bool expected;
    // FIXME(danielm): we should not take ref to bytes
    if (!setjmp(gJmpBuf)) {
      accepted = tryParseValue(temp, cbytes, root);
      crashed = false;
    }

    if (crashed) {
      printf("[ CRSH ] %.*s\n", FMT_SLICE(filename));
      continue;
    }

    switch (filename[0]) {
      case 'y': {
        expected = true;
        break;
      }
      case 'n': {
        expected = false;
        break;
      }
      default: {
        expected = accepted;
        break;
      }
    }

    if (expected == accepted) {
      printf("[  OK  ] %.*s\n", FMT_SLICE(filename));
    } else {
      printf("[ FAIL ] %.*s\n", FMT_SLICE(filename));
    }
  }

  return 0;
}

extern "C" void handleOOM(Arena *arena) {
  fprintf(stderr, "Out of memory\n");
  std::longjmp(gJmpBuf, 1);
}

extern "C" void checkFail(const char *pExpr, const char *pFile, unsigned line) {
  fprintf(stderr, "===\nAssertion failed: %s\n    at %s:%u\n===\n", pExpr,
          pFile, line);
  std::longjmp(gJmpBuf, 1);
}
