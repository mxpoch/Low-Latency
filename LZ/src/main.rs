fn check_subseq(subseq: String, ss_list : &Vec<String>) -> Option<usize> {
    // looking for matches
    let mut count : usize = 0;
    for entry in ss_list {
        if subseq.eq(entry) {
            return Some(count);
        } 
        count += 1;
    } 
     // if no matches are found. 
    return None
}

fn compress(input: &String) -> Vec<(Option<usize>, String)> {
    // declaring the new types
    let mut subseq : Vec<String> = Vec::new();
    let mut cstring : Vec<(Option<usize>, String)> = Vec::new();
    
    // creating sliding window
    let mut window : [usize; 2] = [0,1];

    // entering the loop
    loop {
        // break condition if window extends past the string
        if window[1] > input.len() || window[0] > input.len() {
            // adding the current window to the lists given its a valid window
            // this only happens when the ending is already an entry in subseq.
            if window[0] < input.len() - 1 {
                // adding the prefix
                let prefix = check_subseq(input[window[0]..(window[1]-1)].to_string(), &subseq);
                    match prefix {
                        Some(_x) => (),
                        _ => panic!("Invalid prefix has no match in subseq!"),
                    }
                cstring.push((prefix, String::from("")));
            }   
            break
        } 
        // match sequence if current window already exists as a subseq
        let ss_index = check_subseq(input[window[0]..window[1]].to_string(), &subseq);
        match ss_index {
            
            // already exists in subseq
            Some(_x) => {
                window[1] += 1;
            },

            // does not already exist in subseq
            None => {

                // edge-case where window-size is 1 char
                if window[1] - window[0] == 1 {
                    // adding to lists
                    cstring.push((None, input[window[0]..window[1]].to_string()));
                    subseq.push(input[window[0]..window[1]].to_string());
                } 
                // in case a negative window happens, though it shouldn't
                else if window[1] - window[0] < 1 {
                    panic!("Negative window not permitted!");
                } 
                // general case where both prefix and suffix are > 0
                else {
                    // splitting entry into prefix index and suffix
                    // checking if suffix exists
                    let prefix = check_subseq(input[window[0]..(window[1]-1)].to_string(), &subseq);
                    match prefix {
                        
                        Some(..) => {
                            let suffix = input[(window[1]-1)..(window[1])].to_string();

                            // appending to lists
                            cstring.push((prefix, suffix));
                            subseq.push(input[window[0]..window[1]].to_string());    
                        },
                        
                        _ => panic!("Invalid prefix has no match in subseq!"),
                    }                
                }

                // sliding the window
                window[0] = window[1];
                window[1] += 1;
                }
            }
        }
    println!("{:?}",subseq);
    return cstring
}

fn main() {
    let greeting = String::from("hellohellohellohello");
    // compressing
    let cgreet = compress(&greeting);
    println!("{:?}", cgreet);
    println!("{:?}", cgreet.len());
    println!("{:?}", greeting.len());
}