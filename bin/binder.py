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
ENUM = "@enum"
STRUCT = "@struct"

"""
    rat.registerFn("printHello", print_hello);
    rat.registerFn("addNumbers", add_numbers);
    rat.registerFn("greet", greet);
"""


class Structs:
    fields: dict[list] = {}
    ctr: dict[str] = {}


class Tag:
    op: str
    alias: str
    args: list[str]
    names: list[str]

    def __init__(self):
        self.clear()

    def clear(self):
        self.op = ""
        self.alias = ""
        self.args = []
        self.names = []


def grab_vars(
    tag: Tag,
    binding: dict[str, list],
    class_name: str,
    parts: list[str],
    line: str,
    line_num: int,
):
    b = line.split(";", 1)[0].split("=")
    var = [b for b in b[0].split(" ") if b][-1]
    var = var.replace("&", "").replace("*", "")

    if not var:
        tag.clear()
        print(f"no var found on line {line_num}: {line}")
        return

    binding[class_name].append([tag.op, var, tag.alias if tag.alias else var])


def grab_func(
    tag: Tag,
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
    fn = (
        sig_token.split("(", 1)[0]
        .split("::")[-1]
        .strip()
        .replace("&", "")
        .replace("*", "")
    )

    params = [tag.op, fn, tag.alias if tag.alias else fn, tag.args]
    print(class_name, line, params)
    binding[class_name].append(params)


def get_tag_line(tag, op, line, line_num):
    tag.clear()
    a = line.split(op, 1)
    # print(a)
    if len(a) != 2:
        print(f"can't find {op} on line {line_num}: {line}")
        return False
    tag.op = op

    a = a[1].split("|", 1)
    if len(a) == 2:
        tag.args = a[1]

    # print(a)
    a = a[0].split(":")
    if len(a) != 1:
        tag.alias = a[1]
        tag.names = a[1:]

    # print(a)
    # print(tag)
    return True


def scan_file(
    path: str,
    headers: set[str],
    binding: dict[str, list],
    exports: list[list[str]],
    enums: dict[list],
    structs: Structs,
):
    basename = ntpath.basename(path)
    if basename == "bind.h":
        return

    ALL_CLASS_TAGS = (FUNC, VAR, PROP, CONST, STATICVAR, STATICFUNC)
    with open(path) as sfile:
        struct_name = ""
        enum_group = ""
        class_name = ""
        line_num = 0
        tag: Tag = Tag()
        for line in sfile:
            line_num += 1
            line = line.strip()
            if not line:
                continue
            parts = [p for p in line.split() if p]

            if tag.op:
                headers.add(basename)

            if tag.op == PROP:
                tag.clear()

            # Detect class
            if any(tag in line for tag in ALL_CLASS_TAGS) and not class_name:
                print(f"class_name missing on line {line_num} -- tag ignored: {line}")
                continue

            # class
            elif line.startswith("class ") and not line.endswith(";"):
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
                get_tag_line(tag, EXPORT, line, line_num)
            elif tag.op == EXPORT:
                fn_sig = [p for p in parts if "(" in p]
                if not fn_sig:
                    print(f"can't find {EXPORT} name: {line}")
                    continue
                fn = fn_sig[0].split("(", 1)[0]
                exports.append([fn, tag.alias] if tag.alias else [fn, fn])

            # @enum
            elif ENUM in line:
                get_tag_line(tag, ENUM, line, line_num)
                enum_group = ""
            elif tag.op == ENUM and not enum_group:
                if "enum" not in parts:
                    tag.clear()
                    print(f"missing enum declaration on line {line_num}: {line}")
                    continue
                a = line.strip().split("//", 1)
                a = [x.strip() for x in a[0].split("enum") if x.strip()]
                enum_group = a[0].split(":")[0].strip()
                if not enum_group:
                    tag.clear()
                    print(f"anonymous enums not allowed on line {line_num}: {line}")
                    continue
                if enum_group not in enums:
                    enums[enum_group] = []
            elif tag.op == ENUM and enum_group:
                if line == "{":
                    continue
                elif "};" in parts:
                    enum_group = ""
                    tag.clear()
                    continue
                name = parts[0].split(",")[0]
                enums[enum_group].append([name, f"{class_name}::{enum_group}::{name}"])

            elif STRUCT in line:
                if "typedef" in parts:
                    print(f"typedef not supported on line {line_num}: {line}")
                    continue
                get_tag_line(tag, STRUCT, line, line_num)
            elif tag.op == STRUCT and not struct_name:
                a = line.strip().split("//", 1)
                a = [x.strip() for x in a[0].split("struct") if x.strip()]
                struct_name = a[0].split(":")[0].strip()
                if not struct_name:
                    tag.clear()
                    print(f"anonymous struct not allowed on line {line_num}: {line}")
                    continue
                if struct_name not in structs.fields:
                    structs.fields[struct_name] = []
                    structs.ctr[struct_name] = tag.args
                pass
            elif tag.op == STRUCT and struct_name:
                if line == "{":
                    continue
                elif "};" in parts:
                    tag.clear()
                    struct_name = ""
                    continue
                elif ";" not in line:
                    tag.clear()
                    struct_name = ""
                    continue
                name = parts[-1].split(";")[0]
                structs.fields[struct_name].append(
                    [
                        tag.alias if tag.alias else name,
                        f"{class_name}::{struct_name}::{name}",
                    ]
                )

            elif PROP in line:
                get_tag_line(tag, PROP, line, line_num)
                if not tag.alias:
                    print(f"Malformed {PROP} on line {line_num}: {line}")
                    tag.clear()
                    continue
                parts = tag.alias.split(",")
                if not (2 <= len(parts) <= 3):
                    print(
                        f"{PROP} expects getter,setter[,canSet] on line {line_num}: {line}"
                    )
                    tag.clear()
                    continue
                prop_name = parts[0]
                getter = parts[1]
                setter = parts[2] if len(parts) == 3 else ""
                binding[class_name].append([PROP, prop_name, getter, setter])

            elif FUNC in line:
                get_tag_line(tag, FUNC, line, line_num)

            elif STATICFUNC in line:
                get_tag_line(tag, STATICFUNC, line, line_num)

            elif tag.op in [FUNC, STATICFUNC]:
                grab_func(tag, binding, class_name, parts, line, line_num)
                tag.clear()

            elif VAR in line:
                get_tag_line(tag, VAR, line, line_num)
            elif CONST in line:
                get_tag_line(tag, CONST, line, line_num)
            elif STATICVAR in line:
                get_tag_line(tag, STATICVAR, line, line_num)

            elif tag.op in [VAR, STATICVAR, CONST]:
                grab_vars(tag, binding, class_name, parts, line, line_num)
                tag.clear()


def write_cpp(
    dest: str,
    headers: list[str],
    binding: dict[str, list],
    exports: list[list[str]],
    enums: dict[list],
    structs: Structs,
):
    headers_sorted = list(headers)
    headers_sorted.sort()

    with open(dest, "w") as tfile:
        lines = ["// AUTOGENERATED — DO NOT EDIT", ""]
        lines += ['#include "bind.h"', '#include "treerat.h"', "#include <sqrat.h>"]
        lines += [f'#include "{h}"' for h in headers_sorted]

        #############################################################
        # class bindings
        lines += [
            "",
            "void registerBinding(CTreeRat & rat)",
            "{",
            f"{TAB}HSQUIRRELVM vm = rat.vm();",
        ]
        i = 0
        for class_name, binded in binding.items():
            if not binded:
                continue
            i += 1
            if i != 1:
                lines += [""]
            entity = f"class{class_name}"
            lines += [
                f"{TAB}// === {class_name} Class ===",
                ""
                f'{TAB}auto {entity} = Sqrat::Class<{class_name}>(vm, "{class_name}")',
            ]
            lines += [f"{TAB*2}.Ctor<>()"]

            #############################################################
            # member functions, variables, consts, etc
            for b in binded:
                op = b[0]
                if op == FUNC:
                    ref = f"&{class_name}::{b[1]}"
                    if b[3]:
                        a = b[3].split(",")
                        ref = f"static_cast<{a[0]} ({class_name}::*)({', '.join(a[1:])})>(&{class_name}::{b[1]})"
                    lines += [f'{TAB*2}.Func("{b[2]}", {ref})']
                if op == STATICFUNC:
                    ref = f"&{class_name}::{b[1]}"
                    if b[3]:
                        a = b[3].split(",")
                        ref = f"static_cast<{a[0]} (*)({', '.join(a[1:])})>(&{class_name}::{b[1]})"
                    lines += [f'{TAB*2}.StaticFunc("{b[2]}", {ref})']
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
                f'{TAB}Sqrat::RootTable(vm).Bind("{class_name}", {entity});',
            ]

        #############################################################
        # enums
        for group_name, names in enums.items():
            if not names:
                continue

            table_name = f"enum{group_name}"
            lines += [""]
            lines += [f"{TAB}// === {group_name} Enum ==="]
            lines += [f"{TAB}auto {table_name} = Sqrat::ConstTable(vm)"]
            for name in names:
                naked, ref = name
                lines += [f'{TAB*2}.Const("{naked}", {ref})']

            lines[-1] += ";"
            lines += [f'{TAB}Sqrat::RootTable(vm).Bind("{group_name}", {table_name});']

        #############################################################
        # structs
        """
        // === 2. NOW create Pos class ===
        Sqrat::Class<Pos> posClass(v, "Pos");
        posClass
            .Var("x", &Pos::x)
            .Var("y", &Pos::y);
        Sqrat::RootTable(v).SetValue("Pos", posClass); // overwrite
        """
        for struct_name, names in structs.fields.items():
            if not names:
                continue

            table_name = f"struct{struct_name}"
            lines += [""]
            lines += [f"{TAB}// === {struct_name} Struct ==="]
            lines += [
                f'{TAB}auto {table_name} = Sqrat::Class<{struct_name}>(vm, "{struct_name}")'
            ]
            ctr = structs.ctr[struct_name]
            """
                .Ctor<const std::string&, int>()
            """
            if ctr:
                args = ", ".join(ctr.split(","))
                lines += [f"{TAB*2}.Ctor<{args}>()"]
            for name in names:
                naked, ref = name
                lines += [f'{TAB*2}.Var("{naked}", &{ref})']

            lines[-1] += ";"
            lines += [f'{TAB}Sqrat::RootTable(vm).Bind("{struct_name}", {table_name});']

        lines += ["}"]

        #############################################################
        # global export
        lines += ["", "void registerGlobal(CTreeRat & rat)", "{"]
        for ex in exports:
            lines += [f"""{TAB}rat.registerFn("{ex[1]}", {ex[0]});"""]
        lines += ["}"]

        #############################################################
        # write to file
        tfile.write("\n".join(lines))


def write_header(dest):
    with open(dest, "w") as tfile:
        lines = ["#pragma once", ""]
        lines += ["// AUTOGENERATED — DO NOT EDIT", ""]
        lines += ["class CTreeRat;"]
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
    enums = {}
    structs = Structs
    for path in glob.glob(search_path):
        scan_file(path, headers, binding, exports, enums, structs)

    """

        Sqrat::Class<Entity> entityClass(vm, "Entity");
        entityClass
            .Ctor<>()
            .Func("Move", &Entity::Move)
            .Func("Damage", &Entity::Damage)
            .Func("GetHealth", &Entity::GetHealth);
        Sqrat::RootTable(vm).Bind("Entity", entityClass);

    """

    write_cpp("src/bind.cpp", headers, binding, exports, enums, structs)
    write_header("src/bind.h")


main()
