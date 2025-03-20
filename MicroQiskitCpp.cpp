// g++ -o MicroQiskitCpp MicroQiskitCpp.cpp -DQ_OS_MAC -Ieigen -std=c++11

#include <iostream>
#include <vector>
#include <string>

#ifndef Q_OS_MAC
    #include <QtCore/QLoggingCategory>
#endif

#include "MicroQiskitCpp.h"

using namespace std;

void Qiskit(void) {

    qDebug() << "\n===================================================================================" << Qt::endl;
    qDebug() << "This is MicroQiskitCpp: an extremely minimal version of Qiskit, implemented in C++." << Qt::endl;
    qDebug() << "\nFor the standard version of Qiskit, see qiskit.org. To run your quantum programs\non real quantum hardware, see quantum-computing.ibm.com." << Qt::endl;
    qDebug() << "===================================================================================\n" << Qt::endl;

    qDebug() << "\n**We start with a Bell pair: the standard 'Hello World' of quantum computing.**" << Qt::endl;

    // initialize a circuit with two qubits
    QuantumCircuit qc;
    qc.set_registers(2);
    // add the gates to create a Bell pair
    qc.h(0);
    qc.cx(0,1);

    // initialize another circuit with two qubits and two output bits
    QuantumCircuit meas;
    meas.set_registers(2,2);
    // add the measurements
    meas.measure(0,0);
    meas.measure(1,1);

    // add the measurement circuit to the end of the original circuit
    qc.add(meas);

    // simulate the circuit and get a result
    Simulator result (qc);

    // we can use the result to see what the Qiskit syntax would be
    qDebug() << "\nThis circuit could be expressed in Qiskit as:" << Qt::endl;
    qDebug() << result.get_qiskit() << Qt::endl;

    // we can use the result to see also what the QASM syntax would be
    qDebug() << "\nThis circuit could be expressed in Qasm as:" << Qt::endl;
    qDebug() << result.get_qasm() << Qt::endl;

    // and also to get the counts
    map<string, int> counts = result.get_counts();

    // let's print this to screen
    qDebug() << "The counts are:" << Qt::endl;
    for(map<string, int>::iterator iter = counts.begin(); iter != counts.end(); ++iter){

        string bitstring = iter->first;
        int count = iter->second;

        qDebug() << "Counts for "+bitstring+" = " << count << "\n";

    }

    qDebug() << "\n**Now let's try single qubit rotations and a statevector output.**" << Qt::endl;

    // initialize a circuit with two qubits
    QuantumCircuit qc2;
    qc2.set_registers(2);
    // add some single qubit gates
    qc2.rx(M_PI/4,0);
    qc2.ry(M_PI/2,1);
    qc2.rz(M_PI/8,0);

    // no measurements needed for a statevector output

    // simulate the circuit and get a result
    Simulator result2 (qc2);

    // we can use the result to see what the Qiskit syntax would be
    qDebug() << "\nThis circuit could be expressed in Qiskit as:" << Qt::endl;
    qDebug() << result2.get_qiskit() << Qt::endl;

    qDebug() << "Note: It could also be expressed more efficiently in Qiskit, but MicroQiskit compiles everything down to x, rx, h and cx." << Qt::endl;

    // and also get the statevector
    vector<complex<double> > ket = result2.get_statevector();

    // let's print this to screen
    qDebug() << "\nThe statevector is:" << Qt::endl;
    for (int j=0; j<ket.size(); j++){
        qDebug() << "(" << real(ket[j]) << ") + (" << imag(ket[j]) << ")" <<  "*i" << Qt::endl;
    }

    qDebug() << "\n**Finally, a three qubit GHZ with a memory output.**" << Qt::endl;

    // the circuit
    QuantumCircuit qc3;
    qc3.set_registers(3,3);
    qc3.h(1);
    qc3.cx(1,0);
    qc3.cx(1,2);
    qc3.measure(0,0);
    qc3.measure(1,1);
    qc3.measure(2,2);

    // simulating and getting the result for 10 shots
    Simulator result3 (qc3,100);
    vector<string> memory = result3.get_memory();

    qDebug() << "\nThis circuit could be expressed in Qiskit as:" << Qt::endl;
    qDebug() << result3.get_qiskit() << Qt::endl;

    // we can use the result to see also what the QASM syntax would be
    qDebug() << "\nThis circuit could be expressed in Qasm as:" << Qt::endl;
    qDebug() << result3.get_qasm() << Qt::endl;

    // print to screen
    qDebug() << "The results are:" << Qt::endl;
    for (int j=0; j<memory.size(); j++){
        qDebug() << "Result for sample " << j+1 << " = " << memory[j] << Qt::endl;
    }
}

/*
#ifdef Q_OS_MAC
int main(){
    Qiskit();
}
#endif
*/
