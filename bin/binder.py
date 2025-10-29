import glob
import ntpath
import sys


TAB = " " * 4
FUNC = "@func"
VAR = "@var"
STATICVAR = "@staticvar"
STATICFUNC = "@staticfunc"
PROP = "@prop"
CONST = "@const"
EXPORT = "@export"

"""
    rat.registerFn("printHello", print_hello);
    rat.registerFn("addNumbers", add_numbers);
    rat.registerFn("greet", greet);
"""


def grab_vars(
    binding: dict[str, list],
    class_name: str,
    parts: list[str],
    line: str,
    line_num: int,
):
    b = line.split(";", 1)[0].split("=")
    var = [b for b in b[0].split(" ") if b][-1]
    var = var.replace("&", "").replace("*", "")
    for op in (VAR, CONST, STATICVAR):
        pp = [p for p in parts if p.startswith(op)]
        for p in pp:
            a = p.split(":", 1)
            if len(a) == 1 and not var:
                print(f"{op} name not found on line {line_num}: {line}")
                continue
            elif len(a) == 1:
                binding[class_name].append([op, var, var])
            elif len(a) == 2 and not var:
                # using alias as name
                binding[class_name].append([op, a[1], a[1]])
            elif len(a) == 2:
                binding[class_name].append([op, var, a[1]])
            else:
                print(f"expecting `:alias` for {op} on line {line_num}: {line}")


def grab_func(
    binding: dict[str, list],
    class_name: str,
    parts: list[str],
    line: str,
    line_num: int,
):
    # Find function signature
    sig_token = next((p for p in parts if "(" in p), None)
    if not sig_token:
        print(f"no function signature on line {line_num}: {line}")
        return
    fn = sig_token.split("(", 1)[0].split("::")[-1].strip()
    print(fn)

    # Process each @func
    for tag in (FUNC, STATICFUNC):
        for token in [p for p in parts if p.startswith(tag)]:
            # print(parts)
            a = token.split(":", 1)
            if len(a) == 2:
                rest = a[1]
                alias = rest.split(",", 1)[0].strip() if "," in rest else rest.strip()
            elif len(a) == 1:
                alias = fn
            else:
                print(f"expecting `:alias` for {tag}: {line}")
            op = a[0]

            binding[class_name].append(
                [
                    op,
                    fn,
                    alias,
                ]
            )


def scan_file(
    path: str, headers: set[str], binding: dict[str, list], exports: list[list[str]]
):
    basename = ntpath.basename(path)
    if basename == "bind.h":
        return

    ALL_TAGS = (FUNC, VAR, PROP, CONST, STATICVAR, STATICFUNC)
    with open(path) as sfile:
        class_name = ""
        line_num = 0
        for line in sfile:
            line_num += 1
            line = line.strip()
            if not line or line.startswith("//"):
                continue
            parts = [p for p in line.split() if p]

            # Detect class
            if any(tag in line for tag in ALL_TAGS) and not class_name:
                print(f"class_name missing on line {line_num} -- tag ignored: {line}")
                continue

            elif line.lstrip().startswith("class "):
                class_name = (
                    line.split("class", 1)[1]
                    .split()[0]
                    .split(":")[0]
                    .split("{")[0]
                    .strip()
                )

                if not class_name in binding:
                    binding[class_name] = []
                continue

            # @export
            elif EXPORT in line:
                headers.add(basename)
                fn_sig = [p for p in parts if "(" in p]
                if not fn_sig:
                    print(f"can't find {EXPORT} name: {line}")
                    continue
                fn = fn_sig[0].split("(", 1)[0]
                for ex in [p for p in parts if p.startswith(EXPORT)]:
                    a = ex.split(":", 1)
                    exports.append([fn, a[1]] if len(a) == 2 else [fn, fn])

            # @func
            elif any(tag in line for tag in (FUNC, STATICFUNC)):
                headers.add(basename)
                grab_func(binding, class_name, parts, line, line_num)

            # @var / @const
            elif any(tag in line for tag in (VAR, CONST, STATICVAR)):
                headers.add(basename)
                grab_vars(binding, class_name, parts, line, line_num)

            # @prop
            elif PROP in line:
                headers.add(basename)
                for p in [p for p in parts if p.startswith(PROP)]:
                    a = p.split(":", 1)
                    if len(a) != 2:
                        print(f"Malformed {PROP} on line {line_num}: {line}")
                        continue
                    parts = a[1].split(",")
                    if not (2 <= len(parts) <= 3):
                        print(
                            f"{PROP} expects getter,setter[,canSet] on line {line_num}: {line}"
                        )
                        continue
                    prop_name = parts[0]
                    getter = parts[1]
                    setter = parts[2] if len(parts) == 3 else ""
                    binding[class_name].append([PROP, prop_name, getter, setter])


def write_cpp(
    dest: str, headers: list[str], binding: dict[str, list], exports: list[list[str]]
):
    with open(dest, "w") as tfile:
        lines = ["// AUTOGENERATED — DO NOT EDIT", ""]
        lines += ['#include "bind.h"', '#include "treerat.h"', "#include <sqrat.h>"]
        lines += [f'#include "{h}"' for h in headers]

        # class bindings
        lines += ["", "void registerBinding(CTreeRat & rat)", "{"]
        i = 0
        for class_name, binded in binding.items():
            if not binded:
                continue
            i += 1
            if i != 1:
                lines += [""]
            entity = f"class{class_name}"
            lines += [
                f'{TAB}Sqrat::Class<{class_name}> {entity}(rat.vm(), "{class_name}");'
            ]
            lines += [f"{TAB}{entity}"]
            lines += [f"{TAB*2}.Ctor<>()"]

            # Group overloads
            for b in binded:
                op = b[0]
                if op == FUNC:
                    lines += [f'{TAB*2}.Func("{b[2]}", &{class_name}::{b[1]})']
                if op == STATICFUNC:
                    lines += [f'{TAB*2}.StaticFunc("{b[2]}", &{class_name}::{b[1]})']
                elif op == VAR:
                    lines += [f'{TAB*2}.Var("{b[2]}", &{class_name}::{b[1]})']
                elif op == STATICVAR:
                    lines += [f'{TAB*2}.StaticVar("{b[2]}", &{class_name}::{b[1]})']
                elif op == CONST:
                    lines += [f'{TAB*2}.ConstVar("{b[2]}", &{class_name}::{b[1]})']
                elif op == PROP:
                    getter, setter = b[2], b[3]
                    if setter:
                        lines += [
                            f'{TAB*2}.Prop("{b[1]}", &{class_name}::{getter}, &{class_name}::{setter})'
                        ]
                    else:
                        lines += [f'{TAB*2}.Prop("{b[1]}", &{class_name}::{getter})']

            lines[-1] = lines[-1] + ";"
            lines += [
                f'{TAB}Sqrat::RootTable(rat.vm()).Bind("{class_name}", {entity});',
            ]

        lines += ["}"]

        # global export
        lines += ["", "void registerGlobal(CTreeRat & rat)", "{"]
        for ex in exports:
            lines += [f"""{TAB}rat.registerFn("{ex[1]}", {ex[0]});"""]
        lines += ["}"]

        # write to file
        tfile.write("\n".join(lines))


def write_header(dest):
    with open(dest, "w") as tfile:
        lines = ["#pragma once", ""]
        lines += ["// AUTOGENERATED — DO NOT EDIT", ""]
        lines += ["#include <cstdint>"]
        lines += ["", "class CTreeRat;"]
        lines += [
            "",
            "void registerBinding(CTreeRat & rat);",
            "void registerGlobal(CTreeRat & rat);",
            "",
        ]
        # write to file
        tfile.write("\n".join(lines))


def main():
    if len(sys.argv) > 1:
        search_path = sys.argv[1] + "/*.h"
    else:
        search_path = "src/*.h"

    headers = set()
    binding = {}
    exports = []
    for path in glob.glob(search_path):
        scan_file(path, headers, binding, exports)

    """

        Sqrat::Class<Entity> entityClass(vm, "Entity");
        entityClass
            .Ctor<>()
            .Func("Move", &Entity::Move)
            .Func("Damage", &Entity::Damage)
            .Func("GetHealth", &Entity::GetHealth);
        Sqrat::RootTable(vm).Bind("Entity", entityClass);

    """

    write_cpp("src/bind.cpp", headers, binding, exports)
    write_header("src/bind.h")


main()
