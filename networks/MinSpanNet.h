/*
 * MinSpanNet.h
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

#ifndef MINSPANNET_H_
#define MINSPANNET_H_

#include <vector>
#include <queue>

#include "Vertex.h"
#include "AbstractMSN.h"


class MinSpanNet : public AbstractMSN {
public:
  MinSpanNet(const std::vector<Sequence*> &, const std::vector<bool> &, unsigned = 0);
  virtual ~MinSpanNet();

  
private:

  virtual void computeGraph();
};

#endif /* MINSPANNET_H_ */
