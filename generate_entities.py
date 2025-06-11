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

entities_ir = {}

def add_entity(entity, body, soa):
    global entities_ir
    entities_ir[entity] = {}
    typ_extra = " *" if soa else " "
    if "component" in body:
        for ent_comp in body["component"]:
            if not ent_comp in toml["component"]:
                print("trying to include undefined component '", ent_comp, "' in entity '", entity, "'", sep="")
                exit(1)
            for name, typ in toml["component"][ent_comp].items():
                if (name in entities_ir[entity]) or ("exclude" in body and name in body["exclude"]): continue
                entities_ir[entity][name] = typ + typ_extra
    if "field" in body:
        for name, typ in body["field"].items():
            if type(typ) != str:
                print("fields have to be strings, but the field '", name, "' for entity '", entity, "' isn't", sep="")
                exit(1)
            entities_ir[entity][name] = typ + typ_extra
    if soa:
        entities_ir[entity]["amount"] = "uint32_t "

for entity, body in toml["entity"].items():
    if entity == "unique": continue
    add_entity(entity, body, True)

if "unique" in toml["entity"]:
    for entity, body in toml["entity"]["unique"].items():
        add_entity(entity, body, False)

entities_c = {}
for entity, body in entities_ir.items():
    entities_c[entity] = "struct " + entity + "_data {\n"
    for name, typ in body.items():
        entities_c[entity] += "  " + typ + name + ";\n"
    entities_c[entity] += "};\n"

for entity, body in entities_c.items():
    print(body)

