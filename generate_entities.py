#!/usr/bin/env python3

import tomllib

entities_toml_file = "./entities.toml"

try:
    with open(entities_toml_file, "rb") as f:
        toml = tomllib.load(f)
except FileNotFoundError:
    print("couldn't find '", entities_toml_file, "' file", sep="")
    exit(1)

for component, body in toml["component"].items():
    for name, typ in body.items():
        if type(typ) != str:
            print("fields have to be strings, but the field '", name, "' for component '", component, "' isn't", sep="")
            exit(1)
        if name == "amount":
            print("field 'amount' is reserved for SoA, but component '", component, "' is trying to define it", sep="")
            exit(1)

entities_ir = {}
entities_soa = []
entities_unique = []
entities_all = []

def add_entity(entity, body, soa):
    global entities_ir
    entities_ir[entity] = {}
    if "exclude" in body:
        if type(body["exclude"]) != list:
            print("'exclude' has to be an array, but for entity '", entity, "' it isn't", sep="")
            exit(1)
    if "component" in body:
        if type(body["component"]) != list:
            print("'component' has to be an array, but for entity '", entity, "' it isn't", sep="")
            exit(1)
        for ent_comp in body["component"]:
            if not ent_comp in toml["component"]:
                print("trying to include undefined component '", ent_comp, "' in entity '", entity, "'", sep="")
                exit(1)
            for name, typ in toml["component"][ent_comp].items():
                if (name in entities_ir[entity]) or ("exclude" in body and name in body["exclude"]): continue
                entities_ir[entity][name] = typ
    if "field" in body:
        for name, typ in body["field"].items():
            if type(typ) != str:
                print("fields have to be strings, but the field '", name, "' for entity '", entity, "' isn't", sep="")
                exit(1)
            if name == "amount":
                print("field 'amount' is reserved for SoA, but entity '", entity, "' is trying to define it", sep="")
                exit(1)
            entities_ir[entity][name] = typ
    if soa:
        entities_ir[entity]["amount"] = "uint32_t"
    entities_all.append(entity)

for entity, body in toml["entity"].items():
    if entity == "unique": continue
    add_entity(entity, body, True)
    entities_soa.append(entity)

if "unique" in toml["entity"]:
    for entity, body in toml["entity"]["unique"].items():
        add_entity(entity, body, False)
        entities_unique.append(entity)

entity_headers = {}
for entity, body in entities_ir.items():
    header_guard = "__" + entity.upper() + "_H__"
    entity_headers[entity] = "#ifndef " + header_guard + "\n"
    entity_headers[entity] += "#define " + header_guard + "\n\n"
    entity_headers[entity] += "#include \"yard/math.h\"\n"
    entity_headers[entity] += "#include \"yard/renderer.h\"\n\n"
    entity_headers[entity] += "struct " + entity + "_data {\n"
    is_soa = "amount" in body
    for name, typ in body.items():
        separator = " " if not is_soa or name == "amount" else " *"
        entity_headers[entity] += "  " + typ + separator + name + ";\n"
    entity_headers[entity] += "};\n\n"
    entity_headers[entity] += "void " + entity + "_init(struct " + entity + "_data *self);\n"
    entity_headers[entity] += "void " + entity + "_update(struct " + entity + "_data *self, float dt);\n"
    entity_headers[entity] += "void " + entity + "_render(struct " + entity + "_data *self);\n"
    entity_headers[entity] += "\n#endif/*" + header_guard + "*/\n"

for entity, body in entity_headers.items():
    print(body)

entities_h = "#ifndef __ENTITIES_H__\n"
entities_h += "#define __ENTITIES_H__\n\n"
entities_h += "struct entities_layout {\n"
for entity in entities_soa:
    entities_h += "  uint32_t " + entity + "_amount;\n"
for entity in entities_unique:
    entities_h += "  bool has_" + entity + ";\n"
entities_h += "};\n\n"
entities_h += "bool entities_make(void);\n"
entities_h += "bool entities_layout_set(const struct entities_layout *layout);\n"
entities_h += "void entities_update(float dt);\n"
entities_h += "void entities_render(void);\n"
entities_h += "\n#endif/*__ENTITIES_H__*/\n"

print(entities_h)

entities_c = "#include \"yard/arena.h\"\n"
entities_c += "#include \"yard/entities/entities.h\"\n"
for entity in entities_all:
    entities_c += "#include \"yard/entities/" + entity + ".h\"\n"
entities_c += "\nstruct entities {\n"
entities_c += "  struct arena *arena;\n"
for entity in entities_soa:
    entities_c += "  struct " + entity + "_data " + entity + "_data;\n"
for entity in entities_unique:
    entities_c += "  struct " + entity + "_data *" + entity + "_data;\n"
entities_c += "};\n\n"
entities_c += "static struct entities entities;\n\n"
entities_c += "bool\nentities_make(void) {\n"
entities_c += "  entities.arena = arena_make(0, 0);\n"
entities_c += "  if (!entities.arena) {\n"
entities_c += "    log_errorl(\"couldn't make entities arena\");\n"
entities_c += "    return false;\n"
entities_c += "  }\n"
entities_c += "  log_infol(\"entities manager creation completed!\");\n"
entities_c += "  return true;\n"
entities_c += "}\n\n"
entities_c += "bool\nentities_layout_set(const struct entities_layout *layout) {\n"
entities_c += "  if (!arena_clear(entities.arena)) { log_errorl(\"couldn't clear entities arena\"); return false; }\n"
for entity, body in entities_ir.items():
    if "amount" in body:
        for name, typ in body.items():
            if name == "amount": continue
            entities_c += "  if (layout->" + entity + "_amount) {\n"
            entities_c += "    entities." + entity + "_data." + name + " = arena_push_array(entities.arena, false, " + typ + ", layout->" + entity + "_amount);\n"
            entities_c += "    if (!entities." + entity + "_data." + name + ") {\n"
            entities_c += "      log_errorl(\"couldn't allocate " + entity + " " + name + " data\");\n"
            entities_c += "      return false;\n"
            entities_c += "    }\n"
            entities_c += "  } else {\n"
            entities_c += "    entities." + entity + "_data.amount = 0;\n"
            entities_c += "  }\n"
    else:
        entities_c += "  if (layout->has_" + entity + ") {\n"
        entities_c += "    entities." + entity + "_data = arena_push_type(entities.arena, false, " + typ + ");\n"
        entities_c += "    if (!entities." + entity + "_data) {\n"
        entities_c += "      log_errorl(\"couldn't allocate " + entity + " data\");\n"
        entities_c += "      return false;\n"
        entities_c += "    }\n"
        entities_c += "  } else {\n"
        entities_c += "    entities." + entity + "_data = 0;\n"
        entities_c += "  }\n"
entities_c += "  return true;\n"
entities_c += "}\n\n"
entities_c += "void\nentities_update(float dt) {\n"
entities_c += "#if DEV\n"
entities_c += "  if (!entities.arena) {\n"
entities_c += "    log_errorlf(\"%s: arena is null\", __func__);\n"
entities_c += "    return;\n"
entities_c += "  }\n"
entities_c += "#endif\n"
for entity in entities_soa:
    entities_c += "  if (entities." + entity + "_data.amount) " + entity + "_update(&entities." + entity + "_data, dt);\n"
for entity in entities_unique:
    entities_c += "  if (entities." + entity + "_data) " + entity + "_update(entities." + entity + "_data, dt);\n"
entities_c += "}\n\n"
entities_c += "void\nentities_render(void) {\n"
entities_c += "#if DEV\n"
entities_c += "  if (!entities.arena) {\n"
entities_c += "    log_errorlf(\"%s: arena is null\", __func__);\n"
entities_c += "    return;\n"
entities_c += "  }\n"
entities_c += "#endif\n"
for entity in entities_soa:
    entities_c += "  if (entities." + entity + "_data.amount) " + entity + "_render(&entities." + entity + "_data);\n"
for entity in entities_unique:
    entities_c += "  if (entities." + entity + "_data) " + entity + "_render(entities." + entity + "_data);\n"
entities_c += "}\n"

print(entities_c)
