#!/bin/env ruby
module Data
    def self.get_lambda_data()
        [
            [2, ", i as number, #{$ctx.this}", ", i: number, self: #{$ctx.this_type}"],
            [1, ", i as number", ", i: number"],
            [0, "", ""],
        ]
    end
end
