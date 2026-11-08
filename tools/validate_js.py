import sys
import re

sys.stdout.reconfigure(encoding='utf-8')

def lint_html(file_path):
    print(f"=== LINTING JAVASCRIPT IN {file_path} ===")
    with open(file_path, 'r', encoding='utf-8') as f:
        html = f.read()

    scripts = list(re.finditer(r'<script\b[^>]*>(.*?)</script>', html, re.DOTALL))
    if not scripts:
        print("No <script> tags found.")
        return True

    overall_clean = True

    for s_idx, match in enumerate(scripts):
        js = match.group(1)
        start_offset = match.start(1)
        base_line = html[:start_offset].count('\n') + 1

        stack = []
        i = 0
        n = len(js)
        line = base_line
        col = 1
        errors = []

        while i < n:
            c = js[i]
            if c == '\n':
                line += 1
                col = 1
                i += 1
                continue
            col += 1

            # 1. Single line comment
            if c == '/' and i + 1 < n and js[i+1] == '/':
                i += 2
                while i < n and js[i] != '\n':
                    i += 1
                continue

            # 2. Block comment
            if c == '/' and i + 1 < n and js[i+1] == '*':
                i += 2
                while i + 1 < n and not (js[i] == '*' and js[i+1] == '/'):
                    if js[i] == '\n':
                        line += 1
                        col = 1
                    else:
                        col += 1
                    i += 1
                i += 2
                col += 2
                continue

            # 3. String literals (single & double quotes)
            if c in ('"', "'"):
                quote = c
                q_line = line
                q_col = col
                i += 1
                while i < n:
                    if js[i] == '\\':
                        i += 2
                        col += 2
                        continue
                    if js[i] == quote:
                        i += 1
                        col += 1
                        break
                    if js[i] == '\n':
                        errors.append(f"Unterminated string starting at line {q_line}:{q_col}")
                        break
                    i += 1
                    col += 1
                continue

            # 4. Template literals
            if c == '`':
                t_line = line
                t_col = col
                i += 1
                while i < n:
                    if js[i] == '\\':
                        i += 2
                        col += 2
                        continue
                    if js[i] == '`':
                        i += 1
                        col += 1
                        break
                    if js[i] == '\n':
                        line += 1
                        col = 1
                    else:
                        col += 1
                    i += 1
                continue

            # 5. Regex literals
            # A slash is regex if preceded by punctuation/keywords
            if c == '/':
                # Check previous non-whitespace character
                prev_idx = i - 1
                while prev_idx >= 0 and js[prev_idx] in ' \t\r\n':
                    prev_idx -= 1
                prev_char = js[prev_idx] if prev_idx >= 0 else ''
                if prev_char in '(=[,:!&|?+*-~^;{':
                    # Parse regex
                    r_line = line
                    r_col = col
                    i += 1
                    in_char_class = False
                    while i < n:
                        if js[i] == '\\':
                            i += 2
                            col += 2
                            continue
                        if js[i] == '[':
                            in_char_class = True
                        elif js[i] == ']':
                            in_char_class = False
                        elif js[i] == '/' and not in_char_class:
                            i += 1
                            col += 1
                            # skip regex flags
                            while i < n and js[i].isalnum():
                                i += 1
                                col += 1
                            break
                        if js[i] == '\n':
                            errors.append(f"Unterminated regex literal at line {r_line}:{r_col}")
                            break
                        i += 1
                        col += 1
                    continue

            # 6. Brackets & braces
            if c in '({[':
                stack.append((c, line, col))
            elif c in ')}]':
                if not stack:
                    errors.append(f"Unexpected closing '{c}' at line {line}:{col}")
                    break
                top, t_line, t_col = stack.pop()
                expected = {'(': ')', '{': '}', '[': ']'}[top]
                if c != expected:
                    errors.append(f"Mismatched bracket: expected '{expected}' (opened at line {t_line}:{t_col}), got '{c}' at line {line}:{col}")
                    break

            i += 1

        if stack:
            for top, t_line, t_col in stack:
                errors.append(f"Unclosed '{top}' opened at line {t_line}:{t_col}")

        if errors:
            overall_clean = False
            print(f"❌ SCRIPT #{s_idx} HAS {len(errors)} ERROR(S):")
            for e in errors[:10]:
                print(f"   -> {e}")
        else:
            print(f"✅ SCRIPT #{s_idx}: Perfectly balanced and clean!")

    return overall_clean

if __name__ == '__main__':
    target = sys.argv[1] if len(sys.argv) > 1 else r'c:\Dev\nkai\core\shell.html'
    success = lint_html(target)
    sys.exit(0 if success else 1)
