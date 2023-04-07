extern crate yaml_rust;
use std::env;
use std::fs::File;
use std::io::prelude::*;
use yaml_rust::{YamlLoader, Yaml};

fn main() {
    let args: Vec<String> = env::args().collect();

    let mut input_file = "";
    let mut output_file = "";

    for i in 0..args.len() {
        if args[i] == "-i" || args[i] == "--input" {
            input_file = &args[i + 1];
        } else if args[i] == "-o" || args[i] == "--output" {
            output_file = &args[i + 1];
        }
    }

    if input_file == "" || output_file == "" {
        panic!("Please specify input and output files using -i/--input and -o/--output options");
    }

    let mut file = File::open(input_file).expect("File not found");
    let mut yaml_string = String::new();
    file.read_to_string(&mut yaml_string).expect("Error reading file");

    let docs = YamlLoader::load_from_str(&yaml_string).unwrap();
    let mut binary: Vec<u8> = vec![];

    for doc in docs.iter() {
        let arr = doc.as_vec().unwrap();
        for item in arr.iter() {
            let opcode = item["opcode"].as_i64().unwrap() as u8;
            let num_ops = item["num_ops"].as_i64().unwrap() as u8;

            binary.push(opcode);
            binary.push(num_ops);
        }
    }

    let mut file = File::create(output_file).expect("Error creating file");
    file.write_all(&binary).expect("Error writing file");
}