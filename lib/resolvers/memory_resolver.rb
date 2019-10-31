# frozen_string_literal: true

module Facter
  module Resolvers
    module Linux
      class Memory < BaseResolver
        @semaphore = Mutex.new
        @fact_list ||= {}
        @log = Facter::Log.new
        class << self
          def resolve(fact_name)
            @semaphore.synchronize do
              result ||= @fact_list[fact_name]
              subscribe_to_manager
              result || read_meminfo_file(fact_name)
            end
          end

          def read_meminfo_file(fact_name)
            meminfo_output = File.read('/proc/meminfo')
            read_system(meminfo_output)
            read_swap(meminfo_output)

            @fact_list[fact_name]
          end

          def read_system(output)
            @fact_list[:total] = kilobytes_to_bytes(output.match(/MemTotal:\s+(\d+)\s/)[1])
            @fact_list[:memfree] = kilobytes_to_bytes(output.match(/MemFree:\s+(\d+)\s/)[1])
            @fact_list[:used_bytes] = compute_used(@fact_list[:total], @fact_list[:memfree])
            @fact_list[:capacity] = compute_capacity(@fact_list[:used_bytes], @fact_list[:total])
          end

          def read_swap(output)
            @fact_list[:swap_total] = kilobytes_to_bytes(output.match(/SwapTotal:\s+(\d+)\s/)[1])
            @fact_list[:swap_free] = kilobytes_to_bytes(output.match(/SwapFree:\s+(\d+)\s/)[1])
            @fact_list[:swap_used_bytes] = compute_used(@fact_list[:swap_total], @fact_list[:swap_free])
            @fact_list[:swap_capacity] = compute_capacity(@fact_list[:swap_used_bytes], @fact_list[:swap_total])
          end

          def kilobytes_to_bytes(quantity)
            quantity.to_i * 1024
          end

          def compute_capacity(used, total)
            format('%.2f', (used / total.to_f * 100)) + '%'
          end

          def compute_used(total, free)
            total - free
          end
        end
      end
    end
  end
end