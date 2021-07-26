#include "systemc.h"
#include <cstdlib>
#include <mpi.h>
using namespace std;

class ModuleRank {
public:
    enum {
        Outputter = 0,
        SecondCounter = 1,
        ThirdCounter = 2,
        Inputter = 3
    };
};

int world_size;
int world_rank;

class Data {
public:
    double data_;
    Data() {
        data_ = 0;
    }
    Data(double val) {
        data_ = val;
    }

    ~Data() {
    }
};

SC_MODULE (Inputter) {
    //sc_fifo_out<Data*> out_port;
    sc_port<sc_fifo_out_if<Data*>> out_port;
    sc_export<sc_fifo<Data*>> in_port;
    sc_fifo<Data*> data_fifo;

    void onReceive () {
        Data *input;
        while(data_fifo.nb_read(input)) {
            cout << name() << " receive " << input->data_ << " at " << sc_time_stamp() << endl;
        }
    }

    SC_CTOR(Inputter) {
        in_port(data_fifo);

        SC_METHOD(onReceive);
        dont_initialize();
        sensitive << data_fifo.data_written_event();
    }

};

SC_MODULE (Outputter) {
    sc_port<sc_fifo_out_if<Data*>> out_port;
    sc_export<sc_fifo<Data*>> in_port;
    sc_fifo<Data*> data_fifo;
    sc_in<bool> clk;

    void incr_count () {
        while(true) {
            Data *data = new Data(1);
            cout << name() << " write " << data->data_ << endl;
            out_port->write(data);
            wait();
        }
    }

    void onReceive() {
    }

    SC_CTOR(Outputter) {
        in_port(data_fifo);

        SC_METHOD(onReceive);
        sensitive << data_fifo.data_written_event();

        SC_THREAD(incr_count);
        dont_initialize();
        sensitive << clk.pos();
    }

};

SC_MODULE (Counter) {
public:
    sc_port<sc_fifo_out_if<Data*>> out_port;
    sc_export<sc_fifo<Data*>> in_port;
    sc_fifo<Data*> data_fifo;
    int idx = 0;

    void incr_count () {
        Data *input;
        while(data_fifo.nb_read(input)) {
            cout << name() << " receive " << input->data_ << " at " << sc_time_stamp() << endl;
            while(input->data_ < 10000000000 * idx) {
                input->data_ += std::rand() * 0.00000002;
            }
        }
    }

    SC_CTOR(Counter) {
        in_port(data_fifo);
        SC_METHOD(incr_count);
        sensitive << data_fifo.data_written_event();
    }

};

SC_MODULE (MpiConnector) {
public:
    const int num_ports = 4;
    vector<sc_port<sc_fifo_out_if<Data*>>> *out_ports = new vector<sc_port<sc_fifo_out_if<Data*>>>(num_ports);
    vector<sc_export<sc_fifo<Data*>>> *in_ports = new vector<sc_export<sc_fifo<Data*>>>(num_ports);
    vector<sc_fifo<Data*>> *data_fifos = new vector<sc_fifo<Data*>>(num_ports);
    sc_in<bool> clk;

    void onReceive() {
        if (world_rank == ModuleRank::Ouputter) {
        } else if (world_rank == ModuleRank::SecondCounter) {
        } else if (world_rank == ModuleRank::ThirdCounter) {
        } else if (world_rank == ModuleRank::Inputter) {
        }

        //Data *input;
        //for(int i = 0; i < data_fifos->size(); i++) {
        //    while(data_fifos->at(i).nb_read(input)) {
        //        cout << name() << " receive " << input->data_ << " at " << sc_time_stamp() << endl;
        //    }
        //}
    }

    void HandleThread() {
    }

    SC_CTOR(MpiConnector) {
        SC_METHOD(onReceive);
        for(int i = 0; i < num_ports; i++) {
            in_ports->at(i)(data_fifos->at(i));
            sensitive << data_fifos->at(i).data_written_event();
        }
    }

};

int sc_main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);


    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    sc_clock clk("clk", 1, SC_NS);

    Outputter outputter("outputter");
    Inputter inputter("inputter");
    Counter second_counter("second_counter");
    Counter third_counter("third_counter");
    MpiConnector connector("connector");

    connector.clk(clk);
    outputter.clk(clk);
    second_counter.idx = 1;
    third_counter.idx = 2;

    connector.out_ports->at(ModuleRank::Outputter)(outputter.in_port);
    outputter.out_port(connector.in_ports->at(ModuleRank::Outputter));

    connector.out_ports->at(ModuleRank::SecondCounter)(second_counter.in_port);
    second_counter.out_port(connector.in_ports->at(ModuleRank::SecondCounter));

    connector.out_ports->at(ModuleRank::ThirdCounter)(third_counter.in_port);
    third_counter.out_port(connector.in_ports->at(ModuleRank::ThirdCounter));

    connector.out_ports->at(ModuleRank::Inputter)(inputter.in_port);
    inputter.out_port(connector.in_ports->at(ModuleRank::Inputter));

    sc_start(5, SC_NS);

    //cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;

    MPI_Finalize();
    return 0;
}
