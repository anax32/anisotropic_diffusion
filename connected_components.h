class ConnectedComponents
{
public:
  typedef enum
  {
    FOUR,
    EIGHT,
  } Connectivity;

protected:
  const unsigned int  w;
  const unsigned int  h;
  Connectivity    conn;

  bool labelMatch (
    const unsigned int ref,
    const unsigned int trg,
    const unsigned int *label)
  {
    return (label[ref] == label[trg]);
  }

  bool imageMatch (
    const unsigned int ref,
    const unsigned int trg,
    const unsigned char *image)
  {
  //  return (image[ref] == image[trg]);

    return (abs ((float)image[ref]-(float)image[trg]) < 3.1f);
  }

  void minmaxLabel (
    const unsigned int l1,
    const unsigned int l2,
    const unsigned int *label,
    unsigned int& mn,
    unsigned int& mx)
  {
    if (label[l1] < label[l2])
    {
      mn = l1;
      mx = l2;
    }
    else
    {
      mx = l1;
      mn = l2;
    }
  }

public:
  ConnectedComponents (const unsigned int width, const unsigned int height, Connectivity connectivity = FOUR)
    : w (width), h (height), conn (connectivity)
  {}
  virtual ~ConnectedComponents ()
  {}

  const unsigned int produceLabeling (
    unsigned char *image,
    unsigned int *label)
  {
    const unsigned int  NO_LABEL = 0xFFFFFFFF;
    unsigned int    x, y, i;
    unsigned int    c, r;    // count of labels
    unsigned int    *eqs, *cnt, *meq;
    unsigned int    l, j;//, lmn, lmx;
    unsigned int    src, trg[3];

    c = 0;
    r = 0;

    eqs = new unsigned int[w*h];
    meq = new unsigned int[w*h];
    cnt = new unsigned int[w*h];

    for (i=0; i<w*h; i++)
    {
      label[i] = NO_LABEL;
      eqs[i] = NO_LABEL;
      meq[i] = NO_LABEL;
      cnt[i] = 0;
    }

    for (y=0; y<h; y++)
    {
      for (x=0; x<w; x++)
      {
        src = (y*w)+x;

        trg[0] = (y*w)+(x-1);    // simple 4
        trg[1] = ((y-1)*w)+x;    // simple 4
        trg[2] = ((y-1)*w)+(x-1);  // simple 8

        // horizontal
        if ((x>0) && (imageMatch (src, trg[0], image)))
        {
          label[src] = label[trg[0]];
        }

        // vertical
        if ((y>0) && (imageMatch (src, trg[1], image)) && (!labelMatch (src, trg[1], label)))
        {
          if (label[trg[1]] < label[src])
          {
            if (label[src] != NO_LABEL)
              eqs[label[src]] = label[trg[1]];

            label[src] = label[trg[1]];
          }
          else
          {
            if (label[trg[1]] != NO_LABEL)
              eqs[label[trg[1]]] = label[src];
          }
        }

        // diagonal
        if ((x>0) && (y>0) && (imageMatch (src, trg[2], image)) && (!labelMatch (src, trg[2], label)))
        {
          if (label[trg[2]] < label[src])
          {
            if (label[src] != NO_LABEL)
              eqs[label[src]] = label[trg[2]];

            label[src] = label[trg[2]];
          }
          else
          {
            if (label[trg[2]] != NO_LABEL)
              eqs[label[trg[2]]] = label[src];
          }
        }

        // request a new label
        if (label[src] == NO_LABEL)
        {
          label[src] = c++;
        }

        // track the count
        cnt[label[src]]++;
      }
    }

    // rename the labels
    // build the list of minimum equivalences
    for (i=0; i<w*h; i++)
    {
    //  l = eqs[i];
      l = i;

      while (eqs[l] != NO_LABEL)
      {
        l = eqs[l];
      }

      meq[i] = l;
    }
    // for each pixel, find the min equivalence
    for (i=0; i<w*h; i++)
    {
      //l = label[i];
      //cnt[l]--;

      //while (eqs[l] != NO_LABEL)
      //{
      //  l = eqs[l];
      //}

      //label[i] = l;
      //cnt[l]++;
      cnt[label[i]]--;
      label[i] = meq[label[i]];
      cnt[label[i]]++;
    }


    fprintf (stdout, "c: %i", c);

    // replace the empty labels with good labels
    unsigned int mn, mx;

    for (i=0; i<c; i++)
    {
      if (cnt[i] == 0)
      {
        // replace the last non-empty label with i
        for (j=c, r=0; j>i; j--)
        {
          if (cnt[j] > 0)
          {
            r = j;
            break;
          }
        }

        if (r==0)
          break;

        // replace the label in the image
        for (j=0; j<w*h; j++)
        {
          if (label[j] == r)
            label[j] = i;
        }

        cnt[i] = cnt[r];
        cnt[r] = 0;
      }
    }

    // count the number of components
    mx = 0;
    mn = 0xFFFFFFFF;

    for (i=0, c=0; i<w*h; i++)
    {
      if (cnt[i] > 0)
      {
        c++;
        if (mx<cnt[i])
          mx=cnt[i];
        if (mn>cnt[i])
          mn=cnt[i];
      }
    }

    fprintf (stdout, ", nc: %i [%i -> %i]\n", c, mn, mx);
    delete[] eqs;
    delete[] meq;
    delete[] cnt;
    return c;
  }
};
