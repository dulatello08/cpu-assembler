extern crate yaml_rust;
use std::fs::File;
use std::io::prelude::*;
use std::env;
use yaml_rust::{YamlLoader, Yaml};

fn main() {
    let args: Vec<String> = env::args().collect();

    let mut input_file = String::new();
    let mut output_file = String::new();
    let mut yaml_string = String::new();

    for i in 0..args.len() {
        if args[i] == "-i" || args[i] == "--input" {
            input_file = args[i + 1].clone();
        } else if args[i] == "-o" || args[i] == "--output" {
            output_file = args[i + 1].clone();
        }
    }

    if input_file == "" || output_file == "" {
        panic!("Please specify input and output files using -i/--input and -o/--output options");
    }

    let mut file = File::open(&input_file).expect("File not found");
    file.read_to_string(&mut yaml_string).expect("Error reading file");

    let docs = YamlLoader::load_from_str(&yaml_string).unwrap_or_else(|_| {
        panic!("Error parsing YAML file")
    });
    if docs.is_empty() {
        panic!("Empty YAML document");
    }
    let doc = &docs[0];

    println!("Parsed YAML document: {:?}", doc.as_hash());

    let mut binary: Vec<u8> = vec![];

    for (key, value) in doc.as_hash().unwrap() {
        let opcode = key.as_str().unwrap();
        let operand_count = value.as_i64().unwrap() as u8;

        binary.push(u8::from_str_radix(&opcode, 16).unwrap());
        binary.push(operand_count);
    }

    let mut file = File::create(&output_file).expect("Error creating file");
    file.write_all(&binary).expect("Error writing file");
}